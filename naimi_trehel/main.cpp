// Standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// SDL
#include "SDL/SDL_net.h"
#include "SDL_assert.h"
// Application
#include "NTDemon.hpp"

#define REGISTRY "registry.txt"

using namespace std;

int main(int argc, char **argv)
{
	/* Initialize SDL_net */
	ASSERT_NET(SDLNet_Init() >= 0, "Initialising SDL Net");

  /* Create the Demon */
  NTDemon demon(REGISTRY);

  /* Launch the Demon */
  demon.start();

  /* When the Demon finishes it's program, clean up */
	SDLNet_Quit();

  /* All clear ! */
	return EXIT_SUCCESS;
}
