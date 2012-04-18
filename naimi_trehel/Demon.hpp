#ifndef DEMON_HPP_INCLUDED
#define DEMON_HPP_INCLUDED

#include <list>

#include "SDL/SDL_net.h"

#define REGISTRY "registry.txt"

typedef unsigned int id_t;
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

  /* CLASS VARIABLES */
private:

  /* ATTRIBUTES */
private:
  id_t id;
  UDPsocket socket;
  UDPpacket* packet;
  id_list_t peers;
protected:
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
protected:
  // creation, destruction
  int awaken();
  // communication
<<<<<<< HEAD
  int send(const char* message, unsigned int destination);
  virtual int receive(const char* message, unsigned int source);
=======
  void send(const char* message, id_t destination);
  void broadcast(const char* message);
  void receive(const char* message, id_t source);
>>>>>>> cfd14817be4383517119dec17fced749f5fe5733
};

#endif // DEMON_HPP_INCLUDED
