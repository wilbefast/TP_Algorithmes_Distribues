#ifndef DEMON_HPP_INCLUDED
#define DEMON_HPP_INCLUDED

#include <list>

#include "SDL/SDL_net.h"

#define MAX_DEMONS 1000
#define MAX_FPS 30

// NB - 'id_t' is defined by POSIX as a signed integer
typedef std::list<id_t> id_list_t;
typedef id_list_t::iterator id_list_it;

class Demon
{
  /* NESTING */
protected:
  enum State
  {
    ASLEEP,
    NORMAL,
    ERROR,
    SHUTDOWN
  };

  /* ATTRIBUTES */
private:
  // communication
  UDPsocket socket;
  UDPpacket* packet;
  // timing
  int this_tick, next_tick;
protected:
  // identifiers
  id_t id;
  id_list_t peers;
  // automaton
  State state;

  /* METHODS */
protected:
  // creation, destruction - NB: abstract class
  Demon();
  ~Demon();
public:
  // main loop
  void start();

  /* SUBROUTINES */
private:
  // creation, destruction
  int init();
  int register_id();
  int shutdown();
  int unregister_id();
  // main loop
  int run();
  void wait();
protected:
  // creation, destruction
  virtual int awaken();
  // communication
  void send(const char* message, id_t destination);
  void broadcast(const char* message);
  virtual void receive(const char* message, id_t source);
};

#endif // DEMON_HPP_INCLUDED
