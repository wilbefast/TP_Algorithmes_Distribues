#include <iostream>

#include "NaimiTrehelSite.hpp"

#include "wjd_math.hpp"

#define CS_MAX_DURATION 5000
#define CS_PERCENT_CHANCE 1

using namespace std;

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
  // if(!is_requesting && rand()%1000 <= CS_PERCENT_CHANCE)
  //  supplication();
}

void NaimiTrehelSite::treat_input(char input)
{
  switch(input)
  {
    case 's':
      if(!is_requesting)
        supplication();
    break;

    default:
      cout << "unrecognised input '" << input << "'!" << endl;
  }
}

bool NaimiTrehelSite::receive(const char* message, sid_t source)
{
  // standard utility protocols

      printf("message recu!");
    if(!strcmp("hello", message) && has_token)
      send("i_have_token", source);


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

  // get the token from father, thus indirectly from the root
  if(father != -1)
  {
    send("request",father);
    father = -1;

    // wait for the token to arrive
    while(!has_token)
    {
      /// FIXME -- we're not allowed to block here
      printf("Site %d: 'I am waiting for the token'\n", id);
      SDL_Delay(1000);
    }
  }
  // enter critical section
  critical_section();

  // free up critical section
  liberation();
}

void NaimiTrehelSite::critical_section()
{
  printf("Site %d: 'I am entering critical section now'\n", id);

  /* Simulate critical section by waiting for a short duration */
  SDL_Delay(1000); // this was : SDL_Delay(rand() % CS_MAX_DURATION);
}

void NaimiTrehelSite::liberation()
{
  printf("Site %d: 'I am leaving critical section now'\n", id);
printf("i'm out of CS");
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
  if (is_requesting)
  {
    if (next == -1)
        next = source;
  }
  /* Request token from father */
  if(father != -1)
  {
    send("resquest", source);
  }
  /* Send the token */
  else if(has_token)
  {
      send("token", source);
  }




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
