#include "Demon.hpp"

#include "SDL_assert.h"

#define BASE_PORT 49152
#define LOCALHOST_HEX 0x10007f
#define BASE_PORT_HEX 0xc0
#define PORT_SPACING_HEX 0x100
#define PACKET_SIZE 512

#define ID2PORT(id) BASE_PORT_HEX+id*PORT_SPACING_HEX
#define PORT2ID(port) (port-BASE_PORT_HEX)/PORT_SPACING_HEX


/* CREATION & DESTRUCTION */

Demon::Demon(unsigned int _id) :
id(_id),
socket(),
packet(NULL),
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
	/* Open a socket on the port corresponding to our identifier */
	ASSERT_NET(socket = SDLNet_UDP_Open(BASE_PORT+id), "Opening UDP socket");

	/* Allocate memory for the packet */
	ASSERT(packet = SDLNet_AllocPacket(PACKET_SIZE),
          "Allocating memory for packet");

  /* All clear! */
  return EXIT_SUCCESS;
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
  /* Clean up */
	SDLNet_FreePacket(packet);
	SDLNet_UDP_Close(socket);
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
  /* Send ping */
  send("bink", id);

  /* Check inbox */
  if (SDLNet_UDP_Recv(socket, packet))
    receive((char*)packet->data, PORT2ID(packet->address.port));

  /* All clear ! */
  return EXIT_SUCCESS;
}


/* COMMUNICATION */

int Demon::send(const char* message, unsigned int destination)
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

  /* All clear ! */
  return EXIT_SUCCESS;
}

int Demon::receive(const char* message, unsigned int source)
{
  /* Generic reception call which does nothing special */
  printf("Demon %d received message '%s' from Demon %d\n", id, message, source);

  /* All clear ! */
  state = SHUTDOWN;
  return EXIT_SUCCESS;
}

