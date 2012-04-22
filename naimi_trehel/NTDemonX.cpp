#include "NTDemonX.hpp"

#include "wjd_math.hpp"

#define CS_DURATION 3000

NTDemonX::NTDemonX() :
Demon(),
has_token(false),
is_requesting(false),
father(-1),
next(-1)
{
}

int NTDemonX::awaken()
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
    broadcast("new");
  }

  /* All clear ! */
  return EXIT_SUCCESS;
}

void NTDemonX::receive(const char* message, sid_t source)
{
  // standard utility protocols
  Demon::receive(message, source);

  // received a request or the critical section
  if(!strcmp("request", message))
  {

  }
}

/* SUBROUTINES */

void NTDemonX::supplication()
{
  // requesting on behalf of self, not a different site
  is_requesting = true;

  // get the token from father, thus indirectly from the root
  if(father != -1)
  {
    send("request",father);
    father = -1;
  }
}

void NTDemonX::critical_section()
{
  SDL_Delay(CS_DURATION);
}

void NTDemonX::liberation()
{
}

void NTDemonX::receive_request(sid_t source)
{
  // if this is the tail of the queue, add the requester behind it
  if(is_requesting && next == -1)
    next = source;

  // if this is the root of the tree, the requester becomes to new root
  if(has_token && father == -1)
  {
    has_token = false;
    send("token", source);
    father = source;
  }
  // otherwise ask the current root for the token on behalf of the requester
  else
    send("request", father);
}

void NTDemonX::receive_token(sid_t source)
{
  // this site is now the root of the tree
  has_token = true;

  // perform critical section if the token was requested for the site itself
  if(is_requesting)
  {
    critical_section();
    liberation();
    is_requesting = false;
  }
}
