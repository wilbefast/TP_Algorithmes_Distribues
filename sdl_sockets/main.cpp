#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL/SDL_net.h"
#include "SDL_assert.h"

#define LOCALHOST "127.0.0.1"

int main(int argc, char **argv)
{
	/* Initialize SDL_net */
	ASSERT_NET(SDLNet_Init() >= 0, "Initialising SDL Net");

	/* Open a socket on random port */
  UDPsocket socket;
	ASSERT_NET(socket = SDLNet_UDP_Open(0), "Opening UDP socket");

	/* Resolve server name  */
	IPaddress server;
	ASSERT_NET(SDLNet_ResolveHost(&server, LOCALHOST, 1234) != -1,
              "Resolving host");

	/* Allocate memory for the packet */
	UDPpacket* packet;
	ASSERT(packet = SDLNet_AllocPacket(512), "Allocating memory for packet");

	/* Main loop */
	bool stop = false;
	while (!stop)
	{
		printf("Fill the buffer\n>");
		scanf("%s", (char *)packet->data);

		packet->address.host = server.host;	/* Set the destination host */
		packet->address.port = server.port;	/* And destination port */

		packet->len = strlen((char *)packet->data) + 1;
		SDLNet_UDP_Send(socket, -1, packet); /* This sets the packet->channel */

		/* Quit if packet contains "quit" */
		if (!strcmp((char *)packet->data, "quit"))
			stop = true;
	}

	SDLNet_FreePacket(packet);
	SDLNet_Quit();

	return EXIT_SUCCESS;
}
