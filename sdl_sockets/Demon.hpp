#ifndef DEMON_HPP_INCLUDED
#define DEMON_HPP_INCLUDED

#include "SDL/SDL_net.h"

class Demon
{
  /* NESTING */
public:
  enum State
  {
    ASLEEP,
    NORMAL,
    ERROR
  };

  /* CLASS VARIABLES */
private:
  //static IPaddress localhost;

  /* ATTRIBUTES */
private:
  unsigned int id;
  UDPsocket socket;
  UDPpacket* packet;
protected:
  State state;

  /* METHODS */
//protected:
public: /// FIXME
  // creation, destruction - NB: abstract class
  Demon(unsigned int _id);
  ~Demon();
public:
  // main loop
  void start();

  /* SUBROUTINES */
private:
  // creation, destruction
  int init();
  // main loop
  int run();
protected:
  // creation, destruction
  int awaken();
  // communication
  int send(const char* message, unsigned int destination);
  int receive(const char* message, unsigned int source);
};

#endif // DEMON_HPP_INCLUDED
