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

/* FUNCTIONS */

sid_map_it back(sid_map_t& map)
{
  sid_map_it result = map.end();
  result--;
  return result;
}

void pop_back(sid_map_t& map)
{
  sid_map_it b = back(map);
  map.erase(b);
}

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

void SafeNaimiTrehelSite::awaken()
{
  // standard stuff
  NaimiTrehelSite::awaken();

  // token holder is at position 0
  queue_position = 0;
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
    if(next != -1 && source != next)
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
    logger->write("queued after Site %d", source);

    // the first id is that of the token-holder
    int position = 0;
    data_list_it it = message_data.begin();
    while(it != message_data.end())
    {
      position = (*it);
      it++;
      predecessors[position] = (*it);
    }
    // the last id is the predecessor of the source of the message
    predecessors[position] = source;
    queue_position = position + 1;
    // start checking the predecessor at regular intervals
    check_timer = TIMEOUT;
  }

  else if(s_message.find(ARE_YOU_BEFORE) != string::npos)
  {
    int position = message_data.front();
    if(queue_position < position)
      send_data(QUEUE_POSITION, source, 1, queue_position);
  }

  else if(s_message.find(QUEUE_POSITION) != string::npos)
  {
    //int position = message_data.front();
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
  stringstream s;
  s << temp << ':';

  // the first id is that of the token-holder
  for(sid_map_it i = predecessors.begin(); i != predecessors.end(); i++)
    // numbers are alternating position and site id
    s << i->first << ',' << i->second << ',';

  // inform the queued site that it is now behind this site in the queue
  send(s.str().c_str(), _next);
}

void SafeNaimiTrehelSite::print_info()
{
  // Mother classes' basic information
  NaimiTrehelSite::print_info();

  // Site predecessors
  cout << "predecessors = [ ";
  for(sid_map_it i = predecessors.begin(); i != predecessors.end(); i++)
    cout << "(" << i->second << " @ " << i->first << ") ";
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
    send(ARE_YOU_ALIVE, back(predecessors)->second);
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

    case RECONNECT_QUEUE:
      // time's up for reconnecting the queue
      timeout_reconnect();
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
    logger->write("predecessor %d seems to have crashed",
                  back(predecessors)->second);
    pop_back(predecessors);
    check_timer = 0;
  }
  // if all the predecessors are gone we'll have to reconnect the queue
  else
  {
    logger->write("no more predecessors, attempting to reconnect");
    broadcast_data(ARE_YOU_BEFORE, 1, queue_position);
    reply_timer = 2 * TIMEOUT;
    mechanism = RECONNECT_QUEUE;
  }
}

void SafeNaimiTrehelSite::timeout_reconnect()
{
  // if nobody replied regenerate the token
  if(predecessors.empty())
    regenerate_token();
  // otherwise return to business as usual
  else
    mechanism = POLL_PREDECESSORS;
}
