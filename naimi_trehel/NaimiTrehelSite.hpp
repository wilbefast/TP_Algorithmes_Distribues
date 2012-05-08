#ifndef NAIMITREHELDEMON_HPP_INCLUDED
#define NAIMITREHELDEMON_HPP_INCLUDED

#include "Site.hpp"

// Extended Naimi Trehel Site

class NaimiTrehelSite : public Site
{
  /* ATTRIBUTES */
private:
  bool has_token;
  unsigned int cs_timer;
  sid_t father;
  sid_t next;

  /* METHODS */
public:
  // creation, destruction
  NaimiTrehelSite();
protected:
  // overrides
  void awaken();
  void run();
  bool treat_input(char input);
  bool receive(const char* message, sid_t source);
private:
  // subroutines
  void supplication();
  void critical_section();
  void liberation();
  void receive_request(sid_t source);
  void receive_token(sid_t source);
  void send_token(sid_t destination);
};

#endif // NAIMITREHELDEMON_HPP_INCLUDED
