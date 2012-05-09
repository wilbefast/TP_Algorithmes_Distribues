// Standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
// SDL
#include "SDL/SDL_net.h"
#include "SDL_assert.h"
// Application
#include "SafeNaimiTrehelSite.hpp"

using namespace std;

int main(int argc, char **argv)
{
  // Initialize SDL (needed for timing)
  ASSERT_SDL(SDL_Init(0) >= 0, "Initialising SDL");

	// Initialize SDL_net
	ASSERT_NET(SDLNet_Init() >= 0, "Initialising SDL Net");

  // Create the Site
	SafeNaimiTrehelSite site;

  // Launch the Site
  site.start();

  // When the Site finishes it's program, clean up
	SDLNet_Quit();

  // All clear !
	return EXIT_SUCCESS;
}
