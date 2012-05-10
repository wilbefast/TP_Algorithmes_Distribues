#ifndef SAFENAIMITREHELSITE_HPP_INCLUDED
#define SAFENAIMITREHELSITE_HPP_INCLUDED

#include <string>
#include <map>

#include "NaimiTrehelSite.hpp"

typedef std::map<int, sid_t> sid_map_t;
typedef sid_map_t::iterator sid_map_it;

class SafeNaimiTrehelSite : public NaimiTrehelSite
{
  /* CONSTANTS */
private:
  static const int TIMEOUT = 3*MAX_HERTZ; // maximum latency = 3 seconds

  /* NESTING */
private:
  enum RecoveryMechanism
  {
    POLL_PREDECESSORS,
    RECONNECT_QUEUE,
  };

  /* ATTRIBUTES */
private:
  sid_list_t predecessors;
  sid_map_t _predecessors;
  int check_timer;
  int reply_timer;
  int queue_position;
  RecoveryMechanism mechanism;

  /* METHODS */
public:
  SafeNaimiTrehelSite();

protected:
  // overrides
  void run();
  bool receive(const char* message, sid_t source);
  void queue(sid_t _next);
  void print_info();
  // main methdods
  void liberation();
private:
  // subroutines
  const char* mechanism_to_cstr();
  // fault detection queries
  void poll();
  void poll_predecessors();
  // fault detection replies
  void timeout();
  void timeout_predecessors();
  void timeout_reconnect();
};

#endif // SAFENAIMITREHELSITE_HPP_INCLUDED
