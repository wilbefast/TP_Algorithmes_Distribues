#include "SafeNaimiTrehelSite.hpp"

#include <iostream>
#include <sstream>
#include <string>     // for parsing messages

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
      send("are_you_alive", predecessors.back());
      reply_timer = TIMEOUT;
    }
  }

  // wait for predecessor's reply
  if(reply_timer > 0)
    reply_timer--;
  else if (reply_timer == 0)
  {
    fault = PRED_FAULTY;
    predecessors.pop_back();
    send("are_you_alive", predecessors.back());
    reply_timer = TIMEOUT;
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

  // check for 'are you alive?' message
  if(!s_message.compare("are_you_alive"))
  {
    send("i_am_alive", source);
    if(source != next)
      source = next;
  }

  else if(!s_message.compare("i_am_alive"))
  {
    // check again shortly
    check_timer = TIMEOUT;
    reply_timer = -1;
  }

  // check for commit message (confirmation that we are queued up)
  else if(s_message.find("predecessors:") != string::npos)
  {
    // the first id is that of the token-holder
    add_predecessors(s_message.substr(s_message.find(':')+1));
    // the last id is the predecessor of the source of the message
    predecessors.push_back(source);
    queue_position = predecessors.size();
    // start checking the predecessor at regular intervals
    check_timer = TIMEOUT;
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
  string temp("predecessors:");
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
