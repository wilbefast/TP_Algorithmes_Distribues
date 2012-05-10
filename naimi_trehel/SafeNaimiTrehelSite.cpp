#include "SafeNaimiTrehelSite.hpp"

#include <iostream>
#include <sstream>
#include <string>     // for parsing messages

// pro-tip: use hash-defines to prevent typos from causing problems
#define ARE_YOU_ALIVE "are_you_alive"
#define I_AM_ALIVE "i_am_alive"
#define PREDECESSORS "predecessors"
#define ARE_YOU_BEFORE "are_you_before"
#define QUEUE_POSITION "queue_position"

using namespace std;

/* CREATION, DESTRUCTION */

SafeNaimiTrehelSite::SafeNaimiTrehelSite() :
NaimiTrehelSite(),
predecessors(),
check_timer(-1),
reply_timer(-1),
queue_position(-1),
mechanism(POLL_PREDECESSORS)
{
}

/* OVERRIDES */

void SafeNaimiTrehelSite::run()
{
  // typical run cycle of the Naimi Trehel algorithm
  NaimiTrehelSite::run();

  // perform additional checks to survive other Sites' crashing

  // send messages at regular intervals
  if(check_timer > 0)
    check_timer--;
  else if(check_timer == 0)
    poll();

  // wait for replies
  if(reply_timer > 0)
    reply_timer--;
  else if(reply_timer == 0)
    timeout();
}

bool SafeNaimiTrehelSite::receive(const char* message, sid_t source)
{
  // check if message is consumed by parent method
  if(NaimiTrehelSite::receive(message, source))
    return true;

  // otherwise ...

  // create a string object for easier manipulation
  string s_message(message);

  if(!s_message.compare(ARE_YOU_ALIVE))
  {
    send(I_AM_ALIVE, source);
    if(source != next)
    {
      logger->write("successor %d seems to have crashed", next);
      next = source;
    }
  }

  else if(!s_message.compare(I_AM_ALIVE))
  {
    // check cleared, check again shortly
    check_timer = TIMEOUT;
    reply_timer = -1;
    mechanism = POLL_PREDECESSORS;
  }

  // check for commit message (confirmation that we are queued up)
  else if(s_message.find(PREDECESSORS) != string::npos)
  {
    // the first id is that of the token-holder
    for(data_list_it i = message_data.begin(); i != message_data.end(); i++)
      predecessors.push_back((*i));
    // the last id is the predecessor of the source of the message
    predecessors.push_back(source);
    queue_position = predecessors.size();
    // start checking the predecessor at regular intervals
    check_timer = TIMEOUT;
  }

  else if(s_message.find(ARE_YOU_BEFORE) != string::npos)
  {
    int position = message_data.front();
    if(queue_position < position)
      send_data(QUEUE_POSITION, source, 1, queue_position);
  }

  // default !
  else
  {
    logger->write("Unknown message \"%s\" from %d'\n", message, source);
    return false;
  }

  // event was consumed
  return true;
}

void SafeNaimiTrehelSite::queue(sid_t _next)
{
  // standard operations
  NaimiTrehelSite::queue(_next);

  // build a message composed of the list of predecessors
  string temp(PREDECESSORS);
  stringstream oss;
  oss << temp << ':';

  // the first id is that of the token-holder
  for(sid_list_it i = predecessors.begin(); i != predecessors.end(); i++)
    oss << (*i) << ',';

  // inform the queued site that it is now behind this site in the queue
  send(oss.str().c_str(), _next);
}

void SafeNaimiTrehelSite::print_info()
{
  // Mother classes' basic information
  NaimiTrehelSite::print_info();

  // Site predecessors
  cout << "predecessors = [ ";
  for(sid_list_it i = predecessors.begin(); i != predecessors.end(); i++)
    cout << (*i) << " ";
  cout << "]" << endl;

  // timers
  cout << "check_timer = " << check_timer << endl;
  cout << "reply_timer = " << reply_timer << endl;

  // position in the distributed queue
  cout << "queue_position = " << queue_position << endl;

  // state of fault-repair we're currently in
  cout << "mechanism = " << mechanism_to_cstr() << endl;
}

const char* SafeNaimiTrehelSite::mechanism_to_cstr()
{
  switch(mechanism)
  {
    case POLL_PREDECESSORS:
      return "POLL_PREDECESSORS";
    case RECONNECT_QUEUE:
      return "RECONNECT_QUEUE";
    default:
      return "UNKNOWN";
  }
}

/* MAIN METHODS */

void SafeNaimiTrehelSite::liberation()
{
  // standard liberation procedure
  NaimiTrehelSite::liberation();

  // also clear predecessors and stop polling
  predecessors.clear();
  check_timer = -1;
  reply_timer = -1;
}


/* POLLING */

void SafeNaimiTrehelSite::poll()
{
  // by default we'll wait for a reply ...
  check_timer = -1;

  // .. to the message we'll send depending on the mechanism
  switch(mechanism)
  {
    case POLL_PREDECESSORS:
      // check if each predecessor is responsive
      poll_predecessors();
    break;

    default:
    break;
  }
}

void SafeNaimiTrehelSite::poll_predecessors()
{
  // if there are still predecessors left try to contact the next one
  if(!predecessors.empty())
    send(ARE_YOU_ALIVE, predecessors.back());
  reply_timer = TIMEOUT;
}


/* TIMEOUT */

void SafeNaimiTrehelSite::timeout()
{
  // don't generate multiple timeouts in a row
  reply_timer = -1;

  // act differently depending on what timed-out
  switch(mechanism)
  {
    case POLL_PREDECESSORS:
      // pop predecessors if they are non-responsive
      timeout_predecessors();
    break;

    default:
    break;
  }
}

void SafeNaimiTrehelSite::timeout_predecessors()
{
  // immediately try the next in the queue, discarding the non-responsive one
  if(!predecessors.empty())
  {
    logger->write("predecessor %d seems to have crashed", predecessors.back());
    predecessors.pop_back();
    check_timer = 0;
  }
  // if all the predecessors are gone we'll have to reconnect the queue
  else
  {
    broadcast_data(ARE_YOU_BEFORE, 1, queue_position);
    reply_timer = 2 * TIMEOUT;
    mechanism = RECONNECT_QUEUE;
  }
}
