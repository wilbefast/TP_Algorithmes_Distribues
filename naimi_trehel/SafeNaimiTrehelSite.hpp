#ifndef SAFENAIMITREHELSITE_HPP_INCLUDED
#define SAFENAIMITREHELSITE_HPP_INCLUDED

#include <string>

#include "NaimiTrehelSite.hpp"

class SafeNaimiTrehelSite : public NaimiTrehelSite
{
  /* ATTRIBUTES */
private:
  sid_list_t predecessors;

  /* METHODS */
public:
  SafeNaimiTrehelSite();

protected:
  // overrides
  bool receive(const char* message, sid_t source);
  void queue(sid_t _next);
  void print_info();

private:
  // subroutines
  void add_predecessors(std::string s);
};

#endif // SAFENAIMITREHELSITE_HPP_INCLUDED
