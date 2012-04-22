#include "NTDemonX.hpp"

#include "wjd_math.hpp"

#define CS_MAX_DURATION 5000
#define CS_PERCENT_CHANCE 1

NTDemonX::NTDemonX() :
Demon(),
has_token(false),
is_requesting(false),
father(-1),
next(-1)
{
}

void NTDemonX::awaken()
{
  state = NORMAL;

  // If this is the first Demon created then it has the token
  if(peers.size() == 0)
  {
    has_token = true;
    printf("Demon %d: 'I have the token'\n", id);
  }

  // Otherwise we need to let the other know of this new Demon
  else
  {
    father = peers.front();
    printf("Demon %d: 'Demon %d has the token'\n", id, father);
  }
}

void NTDemonX::idle()
{
  /* Draw a random number to determine whether to simulate critical section */
  if(!is_requesting && rand()%1000 <= CS_PERCENT_CHANCE)
    supplication();
}

bool NTDemonX::receive(const char* message, sid_t source)
{
  // standard utility protocols
  if(Demon::receive(message, source))
    ;

  // received a request for the critical section
  else if(!strcmp("request", message))
    receive_request(source);

  // received the token
  else if(!strcmp("token", message))
    receive_token(source);

  // default !
  else
  {
    printf("Demon %d: 'Unknown message \"%s\" from %d'\n", id, message, source);
    return false;
  }

  // event was consumed
  return true;
}

/* SUBROUTINES */

void NTDemonX::supplication()
{
  printf("Demon %d: 'I am requesting critical section now'\n", id);

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

void NTDemonX::critical_section()
{
  printf("Demon %d: 'I am entering critical section now'\n", id);

  /* Simulate critical section by waiting for a short duration */
  SDL_Delay(rand() % CS_MAX_DURATION);

  /* Liberate at the end of critical section */
  liberation();
}

void NTDemonX::liberation()
{
  printf("Demon %d: 'I am leaving critical section now'\n", id);

  /* Critical section no longer required */
  is_requesting = false;

  /* Send token to next site in queue */
  if(next != -1)
  {
    send_token(next);
    next = -1;
  }
}

void NTDemonX::receive_request(sid_t source)
{
  /* Queue this requesting site up after self */
  if(is_requesting && next == -1)
    next = source;

  /* Send token to requesting site */
  if(has_token)
  {
    send_token(source);
    father = source;
  }

  /* Request token from father */
  else if(father != -1)
    send("request", father);
}

void NTDemonX::receive_token(sid_t source)
{
  // this site is now the root of the tree
  has_token = true;
  father = -1;

  // perform critical section if the token was requested for the site itself
  if(is_requesting)
    critical_section();
}

void NTDemonX::send_token(sid_t destination)
{
  has_token = false;
  send("token", destination);
}
