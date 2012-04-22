#ifndef DEMON_HPP_INCLUDED
#define DEMON_HPP_INCLUDED

#include <list>

#include "SDL/SDL_net.h"

#define MAX_DEMONS 1000
#define MAX_FPS 30

// NB - 'id_t' is already defined by POSIX as an *unsigned* integer
typedef int sid_t;  // 'signed identifier type'
typedef std::list<sid_t> sid_list_t;
typedef sid_list_t::iterator sid_list_it;

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
  sid_t id;
  sid_list_t peers;
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
  void send(const char* message, sid_t destination);
  void broadcast(const char* message);
  virtual void receive(const char* message, sid_t source);
};

#endif // DEMON_HPP_INCLUDED
