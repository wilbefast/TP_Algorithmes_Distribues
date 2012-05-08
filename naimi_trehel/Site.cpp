// File streaming
#include <iostream>
#include <fstream>

#include "Site.hpp"

#include "SDL_assert.h"
#include "kbhit.hpp"

#define REGISTRY "registry.txt"

#define BASE_PORT 49152
#define LOCALHOST_HEX 0x10007f
#define BASE_PORT_HEX 0xc0
#define PORT_SPACING_HEX 0x100
#define PACKET_SIZE 512

#define ID2PORT(id) BASE_PORT_HEX+id*PORT_SPACING_HEX
#define PORT2ID(port) (port-BASE_PORT_HEX)/PORT_SPACING_HEX

#include <signal.h>

using namespace std;

/* CREATION & DESTRUCTION */

Site::Site() :
socket(),
packet(NULL),
this_tick(0),
next_tick(0),
wait_process(-1),
id(0),
peers(),
state(ERROR)
{
  /* Try to initialise the Site, catch errors */
  if(init() != EXIT_SUCCESS)
    WARN("Site::Site", "Initialisation failed");
  else
    state = ASLEEP;
}

int Site::init()
{
  /* Read and write in the registry */
  ASSERT(register_id() == EXIT_SUCCESS, "Reading/editing registry");

	/* Open a socket on the port corresponding to our identifier */
	ASSERT_NET(socket = SDLNet_UDP_Open(BASE_PORT+id), "Opening UDP socket");

	/* Allocate memory for the packet */
	ASSERT(packet = SDLNet_AllocPacket(PACKET_SIZE),
          "Allocating memory for packet");

  /* All clear! */
  return EXIT_SUCCESS;
}

int Site::register_id()
{
  /* Open in write mode first in order to block other from accessing the file */
  ofstream oregistry(REGISTRY, ios::app);
  ASSERT(oregistry, "Opening registry file-writer");

  /* Open in read mode */
  ifstream iregistry(REGISTRY);
  ASSERT(iregistry, "Opening registry file-reader");

  /* Read the identifiers of the other Sites */
  unsigned int new_peer;
  while(iregistry >> new_peer)
    peers.push_back(new_peer);

  /* choose an identifier not already taken by another site */
  for(id = 0; id < MAX_DEMONS; id++)
  {
    bool in_use = false;

    // check whether the current identifier is in use by a peer
    for(sid_list_it i = peers.begin(); i != peers.end(); i++)
      if((*i) == id)
      {
        // skip this identifier if anybody else is using it
        in_use = true;
        break;
      }

    // stop at the first identifier not used by any of the Site's peers
    if(!in_use)
      break;
  }

  /* Register this identifier in the file */
  oregistry << id << endl;
  printf("This Site believes that it is number %d\n", id);

  /* All clear! */
  return EXIT_SUCCESS;

  // NB - all streams are closed automatically at the end of the block
}

void Site::awaken()
{
  /* Generic wake-up call which does nothing special */
  printf("Site %d woke up\n", id);
  state = IDLE;
}

void Site::idle()
{
  /* Generic idle method which does nothing at all */
}

void Site::treat_input(char input)
{
  /* Generic input interpretation method which does nothing at all */
}

Site::~Site()
{
  printf("Site %d destroyed\n", id);
  kill (wait_process, SIGKILL);

  /* Attempt to clean up the mess */
  WARN_IF(shutdown() != EXIT_SUCCESS, "Site::~Site",
          "Site didn't exit cleanly!");
}

int Site::shutdown()
{
  /* Remove identifier from registry */
  ASSERT(unregister_id() == EXIT_SUCCESS, "Removing identifier from registry");

  /* Clean up SDL - "always succeeds" */
	SDLNet_FreePacket(packet);
	SDLNet_UDP_Close(socket);

	/* All clear! */
	return EXIT_SUCCESS;
}

int Site::unregister_id()
{
  /* Create a temporary copy of the register */
  ofstream otemp("temp.txt");
  ASSERT(otemp, "Opening 'temp.txt' file-writer");

  /* Open in read mode */
  ifstream iregistry(REGISTRY);
  ASSERT(iregistry, "Opening registry file-reader");

  /* Copy all identifiers except this Site's */
  sid_t peer;
  while(iregistry >> peer)
    if(peer != id)
      otemp << peer << endl;

  /* Out with the old, in with the new */
  remove(REGISTRY);
  rename("temp.txt", REGISTRY);

  /* All clear! */
  return EXIT_SUCCESS;
}


/* MAIN LOOP */

void Site::start()
{
  /* Make sure them site has been correctly initialised */
  if(state != ASLEEP)
    WARN_RTN_VOID("Site::start", "Not properly initialised");

  /* Let other sites know that this one has joined the fray */
  broadcast("hello");

  /* Wake up method defined by specific algorithm */
  awaken();

  /* Run until there's a problem or a manual shut-down */
  while(state != ERROR && state != SHUTDOWN)
  {
    // don't use 100% of the CPU !
    wait();

    // check for key-presses
    char input = kbhit();

    // interpret key-presses
    if(input)
    {
      // skip a line
      cout << endl;

      // ask site to shut down
      if(input == 'q')
        state = SHUTDOWN;

      // algorithm-specific controls
      else
        treat_input(input);
    }

    // stop if there's an error
    if(run() != EXIT_SUCCESS)
      state = ERROR;
  }
}

int Site::run()
{
  /* Check inbox */
  if (SDLNet_UDP_Recv(socket, packet))
    receive((char*)packet->data, PORT2ID(packet->address.port));

  /* Run idle method, depending on the specific algorithm */
  idle();

  /* All clear ! */
  return EXIT_SUCCESS;
}

// Regulate the number of frames per second, pausing only if need be
void Site::wait()
{
  // Get the current time-stamp
  this_tick = SDL_GetTicks();

  // If it's not yet time for the next update, wait a will
	if (this_tick < next_tick )
		SDL_Delay(next_tick - this_tick);

  // Calculate when the next update should be
	next_tick = this_tick + (1000/MAX_FPS);
}


/* COMMUNICATION */

void Site::send(const char* message, sid_t destination)
{
  /* Build packet */
  unsigned int length = strlen(message) + 1;
  memcpy ((char*)packet->data, message, length);
  packet->address.host = LOCALHOST_HEX;
  packet->address.port = ID2PORT(destination);
  packet->len = length;

  /* Send packet to destination */
  SDLNet_UDP_Send(socket, -1, packet);
  printf("Site %d: 'I sent \"%s\" to Site %d'\n", id, message, destination);
}

void Site::broadcast(const char* message)
{
  /* Send message to each peer */
  for(sid_list_it i = peers.begin(); i != peers.end(); i++)
    send(message, (*i));
}

bool Site::receive(const char* message, sid_t source)
{
  printf("Site %d: 'I received \"%s\" from Site %d'\n", id, message, source);

  /* Standard utility protocols */

  // A new Site is registring its existence
  if(!strcmp(message, "hello"))
  {
    peers.push_back(source);
    printf("Site %d: 'I added %d as a new peer'\n", id, source);
    return true;  // consume event
  }

  // Event has not been consumed
  return false;
}

