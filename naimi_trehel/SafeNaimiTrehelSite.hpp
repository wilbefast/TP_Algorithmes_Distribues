#ifndef SAFENAIMITREHELSITE_HPP_INCLUDED
#define SAFENAIMITREHELSITE_HPP_INCLUDED

#include <string>

#include "NaimiTrehelSite.hpp"

class SafeNaimiTrehelSite : public NaimiTrehelSite
{
  /* CONSTANTS */
private:
  static const int CHECK_INTERVAL = MAX_FPS; // once per second

  /* ATTRIBUTES */
private:
  sid_list_t predecessors;
  unsigned int check_timer;

  /* METHODS */
public:
  SafeNaimiTrehelSite();

protected:
  // overrides
  void run();
  bool receive(const char* message, sid_t source);
  void queue(sid_t _next);
  void print_info();


private:
  // subroutines
  void add_predecessors(std::string s);
};

#endif // SAFENAIMITREHELSITE_HPP_INCLUDED
