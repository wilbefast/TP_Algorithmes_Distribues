#ifndef NAIMITREHELSITE_HPP_INCLUDED
#define NAIMITREHELSITE_HPP_INCLUDED

#include "Site.hpp"

// Extended Naimi Trehel Site

class NaimiTrehelSite : public Site
{
  /* ATTRIBUTES */
private:
  bool has_token;
  int cs_timer;
protected:
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
  // overriden
  virtual bool receive(const char* message, sid_t source);
  void receive_hello(sid_t source);
  virtual void queue(sid_t _next);
  virtual void print_info();
private:
  // subroutines
  void supplication();
  void critical_section();
  void liberation();
  void receive_request(sid_t source);
  void send_token(sid_t destination);
};

#endif // NAIMITREHELSITE_HPP_INCLUDED
