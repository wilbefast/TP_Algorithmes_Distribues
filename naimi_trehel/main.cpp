// Standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// SDL
#include "SDL/SDL_net.h"
#include "SDL_assert.h"
// Application
#include "NTDemon.hpp"

using namespace std;

int main(int argc, char **argv)
{
	/* Initialize SDL_net */
	ASSERT_NET(SDLNet_Init() >= 0, "Initialising SDL Net");

  /* Create the Demon */
  NTDemon demon;

<<<<<<< HEAD
  /* Create the Demon */
  {
    NTDemon demon(id);
    demon.start();
  }
=======
  /* Launch the Demon */
  demon.start();
>>>>>>> cfd14817be4383517119dec17fced749f5fe5733

  /* When the Demon finishes it's program, clean up */
	SDLNet_Quit();

  /* All clear ! */
	return EXIT_SUCCESS;
}
