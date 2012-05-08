#include "NaimiTrehelSite.hpp"

#include "wjd_math.hpp"

#define CS_MAX_DURATION 5000
#define CS_PERCENT_CHANCE 1

NaimiTrehelSite::NaimiTrehelSite() :
Site(),
has_token(false),
is_requesting(false),
father(-1),
next(-1)
{
}

void NaimiTrehelSite::awaken()
{
  state = NORMAL;

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

void NaimiTrehelSite::idle()
{
  /* Draw a random number to determine whether to simulate critical section */
  if(!is_requesting && rand()%1000 <= CS_PERCENT_CHANCE)
    supplication();
}

bool NaimiTrehelSite::receive(const char* message, sid_t source)
{
  // standard utility protocols
  if(Site::receive(message, source))
  {
    if(!strcmp("hello", message) && has_token)
      send("i_have_token", source);
  }

  // received a message telling us who has the token
  if(!strcmp("i_have_token", message))
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
  printf("Site %d: 'I am requesting critical section now'\n", id);

  // requesting on behalf of self, not a different site
  is_requesting = true;

  // if we already have the token we
  if(has_token)
    critical_section();

  // get the token from father, thus indirectly from the root
  else if(father != -1)
  {
    send("request",father);
    father = -1;
  }
}

void NaimiTrehelSite::critical_section()
{
  printf("Site %d: 'I am entering critical section now'\n", id);

  /* Simulate critical section by waiting for a short duration */
  SDL_Delay(rand() % CS_MAX_DURATION);

  /* Liberate at the end of critical section */
  liberation();
}

void NaimiTrehelSite::liberation()
{
  printf("Site %d: 'I am leaving critical section now'\n", id);

  /* Critical section no longer required */
  is_requesting = false;

  /* Send token to next site in queue */
  if(next != -1)
  {
    send_token(next);
    next = -1;
  }
}

void NaimiTrehelSite::receive_request(sid_t source)
{
  /* Queue this requesting site up after self */
  if(father == -1)
    next = source;

  /* Request token from father */
  else
    send("request", father);

  /* The site requesting the token is now my new father */
  father = source;
}

void NaimiTrehelSite::receive_token(sid_t source)
{
  // this site is now the root of the tree
  has_token = true;

  // perform critical section if the token was requested for the site itself
  if(is_requesting)
    critical_section();
}

void NaimiTrehelSite::send_token(sid_t destination)
{
  has_token = false;
  send("token", destination);
}
