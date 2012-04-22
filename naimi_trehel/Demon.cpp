// File streaming
#include <iostream>
#include <fstream>

#include "Demon.hpp"

#include "SDL_assert.h"

#define REGISTRY "registry.txt"

#define BASE_PORT 49152
#define LOCALHOST_HEX 0x10007f
#define BASE_PORT_HEX 0xc0
#define PORT_SPACING_HEX 0x100
#define PACKET_SIZE 512

#define ID2PORT(id) BASE_PORT_HEX+id*PORT_SPACING_HEX
#define PORT2ID(port) (port-BASE_PORT_HEX)/PORT_SPACING_HEX


using namespace std;

/* CREATION & DESTRUCTION */

Demon::Demon() :
socket(),
packet(NULL),
this_tick(0),
next_tick(0),
id(0),
peers(),
state(ERROR)
{
  /* Try to initialise the Demon, catch errors */
  if(init() != EXIT_SUCCESS)
    WARN("Demon::Demon", "Initialisation failed");
  else
    state = ASLEEP;
}

int Demon::init()
{
  /* Read and write in the registry */
  ASSERT(register_id() == EXIT_SUCCESS,
      "Reading/editing registry");

	/* Open a socket on the port corresponding to our identifier */
	ASSERT_NET(socket = SDLNet_UDP_Open(BASE_PORT+id), "Opening UDP socket");

	/* Allocate memory for the packet */
	ASSERT(packet = SDLNet_AllocPacket(PACKET_SIZE),
          "Allocating memory for packet");

  /* All clear! */
  return EXIT_SUCCESS;
}

int Demon::register_id()
{
  /* Open in write mode first in order to block other from accessing the file */
  ofstream oregistry(REGISTRY, ios::app);
  ASSERT(oregistry, "Opening registry file-writer");

  /* Open in read mode */
  ifstream iregistry(REGISTRY);
  ASSERT(iregistry, "Opening registry file-reader");

  /* Read the identifiers of the other Demons */
  unsigned int new_peer;
  while(iregistry >> new_peer)
    peers.push_back(new_peer);

  /* choose an identifier not already taken by another demon */
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

    // stop at the first identifier not used by any of the Demon's peers
    if(!in_use)
      break;
  }

  /* Register this identifier in the file */
  oregistry << id << endl;
  printf("This Demon believes that it is number %d\n", id);

  /* All clear! */
  return EXIT_SUCCESS;

  // NB - all streams are closed automatically at the end of the block
}

void Demon::awaken()
{
  /* Generic wake-up call which does nothing special */
  printf("Demon %d woke up\n", id);
  state = NORMAL;
}

void Demon::idle()
{
  /* Generic idle method which does nothing at all */
}

Demon::~Demon()
{
  printf("Demon %d destroyed\n", id);

  /* Attempt to clean up the mess */
  WARN_IF(shutdown() != EXIT_SUCCESS, "Demon::~Demon",
          "Demon didn't exit cleanly!");
}

int Demon::shutdown()
{
  /* Remove identifier from registry */
  ASSERT(unregister_id() == EXIT_SUCCESS, "Removing identifier from registry");

  /* Clean up SDL - "always succeeds" */
	SDLNet_FreePacket(packet);
	SDLNet_UDP_Close(socket);

	/* All clear! */
	return EXIT_SUCCESS;
}

int Demon::unregister_id()
{
  /* Create a temporary copy of the register */
  ofstream otemp("temp.txt");
  ASSERT(otemp, "Opening 'temp.txt' file-writer");

  /* Open in read mode */
  ifstream iregistry(REGISTRY);
  ASSERT(iregistry, "Opening registry file-reader");

  /* Copy all identifiers except this Demon's */
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

void Demon::start()
{
  /* Make sure them demon has been correctly initialised */
  if(state != ASLEEP)
    WARN_RTN_VOID("Demon::start", "Not properly initialised");

  /* Let other sites know that this one has joined the fray */
  broadcast("new");

  /* Wake up method defined by specific algorithm */
  awaken();

  /* Run until there's a problem */
  while(state == NORMAL)
  {
    // don't use 100% of the CPU !
    wait();
    // break if there's an error
    if(run() != EXIT_SUCCESS)
      state = ERROR;
  }
}

int Demon::run()
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
void Demon::wait()
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

void Demon::send(const char* message, sid_t destination)
{
  /* Build packet */
  unsigned int length = strlen(message) + 1;
  memcpy ((char*)packet->data, message, length);
  packet->address.host = LOCALHOST_HEX;
  packet->address.port = ID2PORT(destination);
  packet->len = length;

  /* Send packet to destination */
  SDLNet_UDP_Send(socket, -1, packet);
  printf("Demon %d: 'I sent \"%s\" to Demon %d'\n", id, message, destination);
}

void Demon::broadcast(const char* message)
{
  /* Send message to each peer */
  for(sid_list_it i = peers.begin(); i != peers.end(); i++)
    send(message, (*i));
}

bool Demon::receive(const char* message, sid_t source)
{
  printf("Demon %d: 'I received \"%s\" from Demon %d'\n", id, message, source);

  /* Standard utility protocols */

  // A new Demon is registring its existence
  if(!strcmp(message, "new"))
  {
    peers.push_back(source);
    printf("Demon %d: 'I added %d as a new peer'\n", id, source);
    return true;  // consume event
  }

  // Event has not been consumed
  return false;
}

