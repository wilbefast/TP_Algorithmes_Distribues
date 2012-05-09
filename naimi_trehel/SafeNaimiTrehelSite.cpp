#include "SafeNaimiTrehelSite.hpp"

#include <iostream>
#include <sstream>
#include <string>     // for parsing messages

using namespace std;

/* CREATION, DESTRUCTION */

SafeNaimiTrehelSite::SafeNaimiTrehelSite() :
NaimiTrehelSite(),
predecessors()
{
}

/* OVERRIDES */

bool SafeNaimiTrehelSite::receive(const char* message, sid_t source)
{
  // create a string object for easier manipulation
  string s_message(message);

  // check if message is consumed by parent method
  if(NaimiTrehelSite::receive(message, source))
    ;

  // check for commit message (confirmation that we are queued up)
  else if(s_message.find("predecessors:") != string::npos)
  {
    logger->write("BINK BINK \"%s\" from %d'\n", time(NULL), id,
          message, source);
  }

  // default !
  else
  {
    logger->write("Unknown message \"%s\" from %d'\n", time(NULL), id,
          message, source);
    return false;
  }

  // event was consumed
  return true;
}

void SafeNaimiTrehelSite::queue(sid_t _next)
{
  // standard operations
  NaimiTrehelSite::queue(_next);

  cout << "HELLO!" << endl;

  // build a message composed of the list of predecessors
  string temp("predecessors:");
  stringstream oss;
  oss << temp;
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
