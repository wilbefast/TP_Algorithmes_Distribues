#include "SafeNaimiTrehelSite.hpp"

#include <iostream>
#include <sstream>
#include <string>     // for parsing messages

// pro-tip: use hash-defines to prevent typos from causing problems
#define ARE_YOU_BEFORE "are_you_before:"
#define ARE_YOU_ALIVE "are_you_alive"
#define I_AM_ALIVE "i_am_alive"
#define PREDECESSORS "predecessors:"

using namespace std;

/* CREATION, DESTRUCTION */

SafeNaimiTrehelSite::SafeNaimiTrehelSite() :
NaimiTrehelSite(),
predecessors(),
check_timer(-1),
reply_timer(-1),
queue_position(-1),
fault(NONE_DETECTED)
{
}

/* OVERRIDES */

void SafeNaimiTrehelSite::run()
{
  // typical run cycle of the Naimi Trehel algorithm
  NaimiTrehelSite::run();

  // perform additional checks to survive other Sites' crashing
  if(state == REQUESTING)
  {
    // check predecessor at regular intervals
    if(check_timer > 0)
      check_timer--;
    else if (check_timer == 0)
    {
      check_timer = -1;
      if(!predecessors.empty())
      {
        // if there are still predecessors left try to contact the next one
        send(ARE_YOU_ALIVE, predecessors.back());
        reply_timer = TIMEOUT;
      }
      // if all the predecessors are gone we'll have to regenerate the token
      else
      {
        // but first, the queue needs to be rebuilt
        broadcast_number(ARE_YOU_BEFORE, queue_position);
        reply_timer = 2 * TIMEOUT;
      }

    }
  }

  // wait for predecessor's reply
  if(reply_timer > 0)
    reply_timer--;
  else if (reply_timer == 0)
  {
      // try the next in the queue, discarding the non-responsive one
      if(!predecessors.empty())
        predecessors.pop_back();
      check_timer = 0;
      reply_timer = -1;
  }
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
      source = next;
  }

  else if(!s_message.compare(I_AM_ALIVE))
  {
    // check cleared, check again shortly
    check_timer = TIMEOUT;
    reply_timer = -1;
  }

  // check for commit message (confirmation that we are queued up)
  else if(s_message.find(PREDECESSORS) != string::npos)
  {
    // the first id is that of the token-holder
    add_predecessors(s_message.substr(s_message.find(':')+1));
    // the last id is the predecessor of the source of the message
    predecessors.push_back(source);
    queue_position = predecessors.size();
    // start checking the predecessor at regular intervals
    check_timer = TIMEOUT;
  }

  else if(s_message.find(ARE_YOU_BEFORE))
    ;

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
  oss << temp;

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
  cout << "fault = " << fault << endl;
}


void SafeNaimiTrehelSite::add_predecessors(string s)
{
  // recursion base case
  if(s.length() < 1)
    return;

  // add the next predecessor
  size_t comma = s.find(',');
  predecessors.push_back(atoi(s.substr(0, comma).c_str()));

  // recursive call for string processing, LISP style !
  add_predecessors(s.substr(comma+1));
}
