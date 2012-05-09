#include <iostream>

#include "NaimiTrehelSite.hpp"

#include "wjd_math.hpp"

#define CS_TIME_INCREMENT 5*MAX_FPS
#define CS_PERCENT_CHANCE 1

#define TOKEN "token"
#define I_HAVE_TOKEN "i_have_token"
#define REQUEST "request"
#define FORWARD_REQUEST_OF "forward_request_of:"

#define KEY_LIBERATE 'l'
#define KEY_SUPPLICATE 's'
#define KEY_TIME 't'

using namespace std;

NaimiTrehelSite::NaimiTrehelSite() :
Site(),
has_token(false),
cs_timer(-1),
father(-1),
next(-1)
{
}

void NaimiTrehelSite::awaken()
{
  state = IDLE;

  // If this is the first Site created then it has the token
  if(peers.size() == 0)
  {
    has_token = true;
    logger->write("I have the token");
  }

  // Otherwise we need to find out who does have the token
  else
  {
    father = peers.front();
    logger->write("Site %d is assumed to have the token", father);
  }
}

void NaimiTrehelSite::run()
{
  // call super-class's generic main loop method
  Site::run();

  switch(state)
  {
    case REQUESTING:
      // if requesting critical section, wait for token to arrive
      if(has_token)
        critical_section();
    break;

    case WORKING:
      // if in critical section, simulate a task via a timer
      if(cs_timer > 0)
      {
        cs_timer--;
        if(!(cs_timer % MAX_FPS))
          logger->write("liberating critical section in %d second(s)",
                      cs_timer/MAX_FPS);
      }
      else if (cs_timer == 0)
        liberation();
    break;

    default:
    break;
  }
}

bool NaimiTrehelSite::treat_input(char input)
{
  // call super-class's generic method - check if it consumes the event
  if(Site::treat_input(input))
    return true;

  // if not check sub-class specific commands
  switch(input)
  {
    // SUPPLICATION
    case KEY_SUPPLICATE:
      cout << "SUPPLICATION" << endl;
      if(state == REQUESTING)
        logger->write("already supplicating");
      else if(state == WORKING)
        logger->write("can't supplicate while in critical section");
      else
        supplication();
      // consume event
      return true;

    // ADD TIME TO CRITICAL SECTION
    case KEY_TIME:
      cout << "TIMER" << endl;
      cs_timer += CS_TIME_INCREMENT;
      logger->write("liberation timer set to %ds", cs_timer/MAX_FPS);
      return true;

    // LIBERATION
    case KEY_LIBERATE:
      cout << "LIBERATION" << endl;
      if(state == WORKING)
        liberation();
      else
        logger->write("can't liberate while not in critical section");
      // consume event
      return true;

    default:
      // don't consume event
      return false;
  }
}

/* OVERRIDDEN */

bool NaimiTrehelSite::receive(const char* message, sid_t source)
{

  // check if message is consumed by parent method
  if(Site::receive(message, source))
    return true;

  // otherwise ...

  // create a string object for easier manipulation
  string s_message(message);

  // received a message telling us who has the token
  if(!s_message.compare(I_HAVE_TOKEN))
  {
    father = source;
    logger->write("Site %d is known to have the token", father);
  }

  // received a request for the critical section
  else if(!s_message.compare(REQUEST))
    receive_request(source);

  // received the token
  else if(!s_message.compare(TOKEN))
    // the presence of the token will be detected in the main 'run' loop and
    // so if requesting the site will enter the critical section
    has_token = true;

  // request forwarded on from another site
  else if(s_message.find(FORWARD_REQUEST_OF) != string::npos)
  {
    sid_t origin = atoi(s_message.substr(s_message.find(':')+1).c_str());

    // keep forwarding on the message until it reaches the root of the tree
    if(father == -1)
      receive_request(origin);
    else
      send_data(FORWARD_REQUEST_OF, father, 1, origin);
  }

  // default !
  else
  {
    // don't consume event
    return false;
  }

  // event was consumed
  return true;
}

void NaimiTrehelSite::receive_hello(sid_t source)
{
  // add this new Site to the list of known peers
  Site::receive_hello(source);

  // also let the new Site known if this Site has the token
  if(has_token)
    send(I_HAVE_TOKEN, source);
}

void NaimiTrehelSite::queue(sid_t _next)
{
  next = _next;
  logger->write("Site %d is now queued after me", _next);
}

void NaimiTrehelSite::print_info()
{
  // print generic Site information
  Site::print_info();

  // Does this site have the token ?
  cout << "has_token = " << has_token << endl;

  // Critical section timer
  cout << "cs_timer = " << cs_timer << endl;

  // Father (in the tree)
  cout << "father = " << father << endl;

  // Next (in the queue)
  cout << "next = " << next << endl;
}


/* SUBROUTINES */

void NaimiTrehelSite::supplication()
{
  // requesting on behalf of self, not a different site
  state = REQUESTING;

  // get the token from father, thus indirectly from the root
  if(father != -1)
  {
    send(REQUEST, father);
    father = -1;
  }

  // state is now REQUESTING : we'll enter the critical section when the token
  // arrives (see NaimiTrehelSite::run).
}

void NaimiTrehelSite::critical_section()
{
  logger->write("I am entering critical section now");

  // Simulate critical section by waiting for a short duration
  state = WORKING;
}

void NaimiTrehelSite::liberation()
{
  logger->write("I am leaving critical section now");

  /* Critical section no longer required */
  state = IDLE;
  cs_timer = -1;

  /* Send token to next site in queue */
  if(next != -1)
  {
    send_token(next);
    next = -1;
  }
}

void NaimiTrehelSite::receive_request(sid_t source)
{
  // If requesting already and not already in the critical section
  if (state == REQUESTING || state == WORKING)
  {
    // queue the sender behind us... unless someone else already called shotgun
    if (next == -1)
      queue(source);
  }

  // Forward request on to father if we have one
  if(father != -1)
    send_data(FORWARD_REQUEST_OF, father, 1, source);

  // Send the token if we're not using it
  else if(has_token && state != WORKING)
    send_token(source);

  // The site requesting the token is now my new father
  father = source;
}

void NaimiTrehelSite::send_token(sid_t destination)
{
  has_token = false;
  send(TOKEN, destination);
}
