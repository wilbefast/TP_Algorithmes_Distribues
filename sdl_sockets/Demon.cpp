#include "Demon.hpp"

#include "SDL_assert.h"

#define LOCALHOST "127.0.0.1"
#define FIRST_PORT 49152
#define PACKET_SIZE 512

/* CREATION & DESTRUCTION */

Demon::Demon(unsigned int _id) :
id(_id),
socket(),
packet(NULL)
{
  WARN_IF(init() != EXIT_SUCCESS, "Demon::Demon", "Failed to initialise Demon");
}

int Demon::init()
{
	/* Open a socket on the port corresponding to our identifier */
	ASSERT_NET(socket = SDLNet_UDP_Open(id + FIRST_PORT),
          "Opening UDP socket");

	/* Allocate memory for the packet */
	ASSERT(packet = SDLNet_AllocPacket(PACKET_SIZE),
          "Allocating memory for packet");

  /* All clear! */
  return EXIT_SUCCESS;
}

int Demon::awaken()
{
  WARN("Demon::awaken", "Default (null) implementation was called");
  return EXIT_SUCCESS;
}

Demon::~Demon()
{
  /* Clean up */
	SDLNet_FreePacket(packet);
	SDLNet_UDP_Close(socket);
}


/* MAIN LOOP */

void Demon::start()
{
  /* Wake up method defined by specific algorithm */
  awaken();

  /* Run until there's a problem */
  while(!stop)
    if(run() != EXIT_SUCCESS)
      stop = true;
}

int Demon::run()
{
  /* Check inbox */
  if (SDLNet_UDP_Recv(socket, packet))
    receive(packet->data, packet->address.port);

  /* All clear ! */
  return EXIT_SUCCESS;
}


/* COMMUNICATION */

int Demon::send(Uint8* message, unsigned int destination)
{
  /* Resolve server name */
	IPaddress server;
	ASSERT_NET(SDLNet_ResolveHost(&server, LOCALHOST, destination + FIRST_PORT)
              != -1, "Resolving host");

  /* Build packet */
  packet->address.host = server.host;
  packet->address.port = server.port;
  packet->len = strlen((char *)packet->data) + 1;

  /* Send packet to destination */
  SDLNet_UDP_Send(socket, -1, packet);

  /* All clear ! */
  return EXIT_SUCCESS;
}

int Demon::receive(Uint8* message, unsigned int source)
{
  /* Resolve server name  */
  LOG_I("Received message", message);

  /* All clear ! */
  return EXIT_SUCCESS;
}

