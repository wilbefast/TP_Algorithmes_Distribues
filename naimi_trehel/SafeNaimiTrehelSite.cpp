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
    printf("%ld -- Site %d: 'BINK BINK \"%s\" from %d'\n", time(NULL), id,
          message, source);
  }

  // default !
  else
  {
    printf("%ld -- Site %d: 'Unknown message \"%s\" from %d'\n", time(NULL), id,
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

  // inform the queued site that it is now behind this site in the queue

  string temp("predecessors:");
  stringstream oss;
  oss << temp;

  for(sid_list_it i = predecessors.begin(); i != predecessors.end(); i++)
    oss << (*i) << ',';

  // send the combination to the requested destination
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
