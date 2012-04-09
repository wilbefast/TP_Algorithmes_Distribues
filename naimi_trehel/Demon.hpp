#ifndef DEMON_HPP_INCLUDED
#define DEMON_HPP_INCLUDED

#include <list>

#include "SDL/SDL_net.h"

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
  //static IPaddress localhost;

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
  Demon(const char* registry_file);
  ~Demon();
public:
  // main loop
  void start();

  /* SUBROUTINES */
private:
  // creation, destruction
  int init(const char* registry_file);
  int init_identifiers(const char* registry_file);
  // main loop
  int run();
protected:
  // creation, destruction
  int awaken();
  // communication
  int send(const char* message, id_t destination);
  int receive(const char* message, id_t source);
};

#endif // DEMON_HPP_INCLUDED
