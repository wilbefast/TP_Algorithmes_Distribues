// File streaming
#include <iostream>
#include <fstream>

#include "Demon.hpp"

#include "SDL_assert.h"

#define BASE_PORT 49152
#define MAX_DEMONS 1000
#define LOCALHOST_HEX 0x10007f
#define BASE_PORT_HEX 0xc0
#define PORT_SPACING_HEX 0x100
#define PACKET_SIZE 512

#define ID2PORT(id) BASE_PORT_HEX+id*PORT_SPACING_HEX
#define PORT2ID(port) (port-BASE_PORT_HEX)/PORT_SPACING_HEX


using namespace std;

/* CREATION & DESTRUCTION */

Demon::Demon() :
id(0),
socket(),
packet(NULL),
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
    for(id_list_it i = peers.begin(); i != peers.end(); i++)
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

  /* All clear! */
  return EXIT_SUCCESS;

  // NB - all streams are closed automatically at the end of the block
}

int Demon::awaken()
{
  /* Generic wake-up call which does nothing special */
  printf("Demon %d woke up\n", id);
  state = NORMAL;
  return EXIT_SUCCESS;
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
  id_t peer;
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

  /* Wake up method defined by specific algorithm */
  awaken();

  /* Run until there's a problem */
  while(state == NORMAL)
    if(run() != EXIT_SUCCESS)
      state = ERROR;
}

int Demon::run()
{
  /* Send ping to next peer */
  if(peers.size() > 0)
    send("bink", (id+1)%peers.size());

  /* Check inbox */
  if (SDLNet_UDP_Recv(socket, packet))
    receive((char*)packet->data, PORT2ID(packet->address.port));

  /* All clear ! */
  return EXIT_SUCCESS;
}


/* COMMUNICATION */

void Demon::send(const char* message, id_t destination)
{
  /* Build packet */
  unsigned int length = strlen(message) + 1;
  memcpy ((char*)packet->data, message, length);
  packet->address.host = LOCALHOST_HEX;
  packet->address.port = ID2PORT(destination);
  packet->len = length;

  /* Send packet to destination */
  SDLNet_UDP_Send(socket, -1, packet);
  printf("Demon %d sent message '%s' to Demon %d\n", id, message, destination);
}

void Demon::broadcast(const char* message)
{
  /* Send message to each peer */
  for(id_list_it i = peers.begin(); i != peers.end(); i++)
    send(message, (*i));
}

void Demon::receive(const char* message, id_t source)
{
  /* Generic reception call which does nothing special */
  printf("Demon %d received message '%s' from Demon %d\n", id, message, source);

  /* All clear ! */
  state = SHUTDOWN;
}

