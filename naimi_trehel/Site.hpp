#ifndef DEMON_HPP_INCLUDED
#define DEMON_HPP_INCLUDED

#include <list>

#include "SDL/SDL_net.h"

#define MAX_DEMONS 1000
#define MAX_FPS 30

#define STR_EQ(x, y) !strcmp(x, y)

// NB - 'id_t' is already defined by POSIX as an *unsigned* integer
typedef int sid_t;  // 'signed identifier type'
typedef std::list<sid_t> sid_list_t;
typedef sid_list_t::iterator sid_list_it;

class Site
{
  /* NESTING */
protected:
  enum State
  {
    ASLEEP,
    IDLE,
    WORKING,
    WAITING,
    ERROR,
    SHUTDOWN
  };

  /* ATTRIBUTES */
private:
  // communication
  UDPsocket socket;
  UDPpacket* packet;
  // local timing
  int this_tick, next_tick;
  // synchronisation
  int clock;

protected:
  // fork
  pid_t wait_process;
  // identifiers
  sid_t id;
  sid_list_t peers;
  // automaton
  State state;

  /* METHODS */
protected:
  // creation, destruction - NB: abstract class
  Site();
  ~Site();
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
  virtual void awaken();
  // main loop
  virtual void idle();
  virtual void treat_input(char input);
  // communication
  void send(const char* message, sid_t destination);
  void broadcast(const char* message);
  virtual bool receive(const char* message, sid_t source);
};

#endif // DEMON_HPP_INCLUDED
