#include <iostream>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>

#include "NaimiTrehelSite.hpp"

#include "wjd_math.hpp"

#define CS_MAX_DURATION 5000
#define CS_PERCENT_CHANCE 1

#define FORK_FAILED -1
#define FORK_CHILD 0
#define FORK_PARENT 1

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

bool NaimiTrehelSite::treat_input(char input)
{
  // call super-class's generic method - check if it consumes the event
  if(Site::treat_input(input))
    return true;

  // if not check sub-class specific commands
  switch(input)
  {
    case 's':
      if(!is_requesting)
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
    if(has_token && !strcmp("hello", message))
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
static bool *mem_has_token;
void NaimiTrehelSite::supplication()
{
  // requesting on behalf of self, not a different site
  is_requesting = true;

  // Allocate a shared memory segment for each desired shared variable and save the id
  int memid_has_token = shmget(IPC_PRIVATE, 100, 0700 | IPC_CREAT);
  if (memid_has_token == -1)
  {
      perror("shmget");
      exit (EXIT_FAILURE);
  }

  /* mounting memory segments */
  mem_has_token = (bool*)shmat(memid_has_token, NULL, 0);

  /* affecting the right values to the right variables */
  mem_has_token[0] = has_token;

  printf("Site %d: 'Requesting critical section now'\n", id);
  // get the token from father, thus indirectly from the root
      if(father != -1)
      {
          send("request",father);
          father = -1;

          // wait for the token to arrive
          wait_process = fork();
          switch (wait_process)
          {
            // Fork failed
            case FORK_FAILED:
                perror("fork");
                exit(EXIT_FAILURE); // Critical error -> Shutdown
            break;

            // Child process -- enter critical section, liberate and then die
            case FORK_CHILD:
                while(!mem_has_token[0])
                {
                    printf("Site %d: 'Child process waiting for the token'\n", id);
                    SDL_Delay(1000);
                }
                // kill the child process
                exit(EXIT_SUCCESS);
            break;
            // Parent process -- return to other matters (replying to other sites)
            case FORK_PARENT:
            default:
            //printf("Site %d: 'Parent process returning to other duties'\n", id);
            break;
          }
          // enter critical section
          critical_section();

          // free up critical section
          liberation();
      }
      else
      {
        // enter critical section
        critical_section();

        // free up critical section
        liberation();
      }
}

void NaimiTrehelSite::critical_section()
{
  printf("Site %d: 'I am entering critical section now'\n", id);

  /* Currently in critical section */
  state = WORKING;

  /* Simulate critical section by waiting for a short duration */
  // SDL_Delay(rand() % CS_MAX_DURATION);
  SDL_Delay(1000);
}

void NaimiTrehelSite::liberation()
{
  printf("Site %d: 'I am leaving critical section now'\n", id);

  /* Critical section no longer required */
  is_requesting = false;

  // Destroys the son process, in case he's still alive >>:)
  if (wait_process != -1)
  {
    kill (wait_process, SIGKILL);
    wait_process = -1;
  }

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
  else if(father != -1)
  {
    send("request", source);
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
  mem_has_token[0] = 1;
}

void NaimiTrehelSite::send_token(sid_t destination)
{
  has_token = false;
  send("token", destination);
}
