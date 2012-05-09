#include <iostream>

#include "NaimiTrehelSite.hpp"

#include "wjd_math.hpp"

#define CS_MAX_DURATION 150
#define CS_PERCENT_CHANCE 1

using namespace std;

NaimiTrehelSite::NaimiTrehelSite() :
Site(),
has_token(false),
cs_timer(0),
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
    printf("%ld -- Site %d: 'I have the token'\n", time(NULL), id);
  }

  // Otherwise we need to find out who does have the token
  else
  {
    father = peers.front();
    printf("%ld -- Site %d: 'Site %d is assumed to have the token'\n",
          time(NULL), id, father);
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
      if(cs_timer)
      {
        cs_timer--;
        if(!(cs_timer % MAX_FPS))
          printf("%ld -- Site %d: 'liberating critical section in %d second(s)'\n",
                time(NULL), id, cs_timer/MAX_FPS);
      }
      else
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
    case 's':
      cout << "SUPPLICATION" << endl;
      if(state != REQUESTING && state != WORKING)
      {
        supplication();
        return true;
      }
      else
      {
        printf("Site %d: 'can't supplicate at the moment'\n", id);
        return false;
      }
    break;

    default:
      cout << "UNRECOGNISED input '" << input << "'!" << endl;
      return false;
  }
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

bool NaimiTrehelSite::receive(const char* message, sid_t source)
{
  // create a string object for easier manipulation
  string s_message(message);

  // standard utility protocols
  if(Site::receive(message, source))
  {
    if(has_token && !s_message.compare("hello"))
      send("i_have_token", source);
  }

  /* received a message not consumed by Site::receive */

  // received a message telling us who has the token
  else if(!s_message.compare("i_have_token"))
  {
    father = source;
    printf("%ld -- Site %d: 'Site %d is known to have the token'\n", time(NULL),
          id, father);
  }

  // received a request for the critical section
  else if(!s_message.compare("request"))
    receive_request(source);

  // received the token
  else if(!s_message.compare("token"))
    receive_token(source);

  // request forwarded on from another site
  else if(s_message.find("forward_req_of:") != string::npos)
  {
    sid_t origin = atoi(s_message.substr(s_message.find(':')+1).c_str());
    receive_request(origin);
  }

  // default !
  else
  {
    printf("%ld -- Site %d: 'Unknown message \"%s\" from %d'\n", time(NULL), id,
          message, source);
    return false;
  }

  // event was consumed
  return true;
}

/* SUBROUTINES */

void NaimiTrehelSite::supplication()
{
  // requesting on behalf of self, not a different site
  state = REQUESTING;

  // get the token from father, thus indirectly from the root
  if(father != -1)
  {
    send("request",father);
    father = -1;
  }

  // state is now REQUESTING : we'll enter the critical section when the token
  // arrives (see NaimiTrehelSite::run).
}

void NaimiTrehelSite::critical_section()
{
  printf("%ld -- Site %d: 'I am entering critical section now'\n", time(NULL),
          id);

  // Simulate critical section by waiting for a short duration
  state = WORKING;
  cs_timer = rand() % CS_MAX_DURATION;
}

void NaimiTrehelSite::liberation()
{
  printf("%ld -- Site %d: 'I am leaving critical section now'\n", time(NULL),
        id);

  /* Critical section no longer required */
  state = IDLE;

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
    {
        next = source;
        printf("%ld -- Site %d: 'Site %d is now queued after me'\n", time(NULL),
              id, source);
    }
  }

  // Request token from father
  if(father != -1)
    send_number("forward_req_of:", source, father);

  // Send the token
  else if(has_token)
    send_token(source);

  // The site requesting the token is now my new father
  father = source;
}

void NaimiTrehelSite::receive_token(sid_t source)
{
  // this site is now the root of the tree
  has_token = true;

  // perform critical section if the token was requested for the site itself
  if(state == REQUESTING)
    critical_section();
}

void NaimiTrehelSite::send_token(sid_t destination)
{
  has_token = false;
  send("token", destination);
}
