#include <iostream>
#include <errno.h>

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
    printf("Site %d: 'I have the token'\n", id);
  }

  // Otherwise we need to find out who does have the token
  else
  {
    father = peers.front();
    printf("Site %d: 'Site %d is assumed to have the token'\n", id, father);
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
        printf("Site %d: 'liberating critical section in %d'\n", id, cs_timer);
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
      if(state != REQUESTING && state != WORKING)
        supplication();
        return true;
    break;

    default:
      cout << "unrecognised input '" << input << "'!" << endl;
      return false;
  }
}

bool NaimiTrehelSite::receive(const char* message, sid_t source)
{
  // standard utility protocols
  if(Site::receive(message, source))
  {
    if(has_token && STR_EQ("hello", message))
      send("i_have_token", source);
  }

  /* received a message not consumed by Site::receive */

  // received a message telling us who has the token
  else if(!strcmp("i_have_token", message))
  {
    father = source;
    printf("Site %d: 'Site %d is known to have the token'\n", id, father);
  }

  // received a request for the critical section
  else if(!strcmp("request", message))
    receive_request(source);

  // received the token
  else if(!strcmp("token", message))
    receive_token(source);

  // default !
  else
  {
    printf("Site %d: 'Unknown message \"%s\" from %d'\n", id, message, source);
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
  printf("Site %d: 'I am entering critical section now'\n", id);

  // Simulate critical section by waiting for a short duration
  state = WORKING;
  cs_timer = rand() % CS_MAX_DURATION;
}

void NaimiTrehelSite::liberation()
{
  printf("Site %d: 'I am leaving critical section now'\n", id);

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
  // Queue this requesting site up after self if in or awaiting critical section
  if (state == REQUESTING || state == WORKING)
  {
    if (next == -1)
        next = source;
  }

  // Request token from father
  else if(father != -1)
    send("request", father);

  // Send the token
  else if(has_token)
    send("token", source);

  // The site requesting the token is now my new father
  father = source;
}

void NaimiTrehelSite::receive_token(sid_t source)
{
  // this site is now the root of the tree
  has_token = true;
    printf("token = %d\n", has_token);

  // perform critical section if the token was requested for the site itself
  if(state == REQUESTING)
    critical_section();
}

void NaimiTrehelSite::send_token(sid_t destination)
{
  has_token = false;
  send("token", destination);
}
