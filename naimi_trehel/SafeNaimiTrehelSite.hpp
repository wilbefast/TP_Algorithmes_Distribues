#ifndef SAFENAIMITREHELSITE_HPP_INCLUDED
#define SAFENAIMITREHELSITE_HPP_INCLUDED

#include <string>

#include "NaimiTrehelSite.hpp"

class SafeNaimiTrehelSite : public NaimiTrehelSite
{
  /* CONSTANTS */
private:
  static const int TIMEOUT = MAX_FPS; // maximum latency = 1 second

  /* NESTING */
private:
  enum Fault
  {
    NONE_DETECTED,
    PRED_FAULTY
  };

  /* ATTRIBUTES */
private:
  sid_list_t predecessors;
  int check_timer;
  int reply_timer;
  int queue_position;
  Fault fault;

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
