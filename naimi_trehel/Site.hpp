#ifndef SITE_HPP_INCLUDED
#define SITE_HPP_INCLUDED

#include <list>

#include "SDL/SDL_net.h"

// mutual dependence
class Site;
#include "SiteLogger.hpp"

#define MAX_SITES 1000
#define MAX_FPS 30

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
    REQUESTING,
    WORKING,
    FAULT_RECOVERY,
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
  //int clock;
  /// is this needed ?

protected:
  // identifiers
  sid_t id;
  sid_list_t peers;
  // automaton
  State state;
  // debug information
  SiteLogger* logger;

  /* METHODS */
protected:
  // creation, destruction - NB: abstract class
  Site();
  ~Site();
public:
  // main loop
  void start();
  // query
  sid_t getId() const;

  /* SUBROUTINES */
private:
  // creation, destruction
  int init();
  int register_id();
  int shutdown();
  int unregister_id();
  // main loop
  void wait();
protected:
  // creation, destruction
  virtual void awaken();
  // main loop
  virtual void run();
  // user interface
  virtual bool treat_input(char input);
  virtual void print_info();
  // communication
  void send(const char* message, sid_t destination);
  void send_data(const char* header, sid_t destination, int argc, ...);
  void send_data(const char* header, sid_t destination, int argc,
                va_list arguments);
  void broadcast(const char* message);
  void broadcast_data(const char* header, int argc, ...);
  virtual bool receive(const char* message, sid_t source);
  // overridden communcation
  virtual void receive_hello(sid_t source);
};

#endif // SITE_HPP_INCLUDED
