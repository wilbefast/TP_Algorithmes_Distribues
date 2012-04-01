#ifndef DEMON_HPP_INCLUDED
#define DEMON_HPP_INCLUDED

#include "SDL/SDL_net.h"

class Demon
{
  /* ATTRIBUTES */
private:
  unsigned int id;
  UDPsocket socket;
  UDPpacket* packet;
protected:
  bool stop;

  /* METHODS */
public:
  // creation, destruction
  Demon(unsigned int _id);
  ~Demon();
  // main loop
  void start();

  /* SUBROUTINES */
private:
  // creation, destruction
  int init();
  int awaken();
  // main loop
  int run();
protected:
  // communication
  int send(Uint8* message, unsigned int destination);
  int receive(Uint8* message, unsigned int source);
};

#endif // DEMON_HPP_INCLUDED
