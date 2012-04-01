#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL/SDL_net.h"

#include "SDL_assert.h"
#include "Demon.hpp"

int main(int argc, char **argv)
{
	/* Initialize SDL_net */
	ASSERT_NET(SDLNet_Init() >= 0, "Initialising SDL Net");

  /* Creation the Demon */
  Demon demon(0);
  demon.start();

  /* Clean up */
	SDLNet_Quit();

  /* All clear ! */
	return EXIT_SUCCESS;
}
