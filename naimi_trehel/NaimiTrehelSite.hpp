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
  // query
  bool hasToken() const;
protected:
    // fault recovery
  virtual void regenerate_token();
  // overrides
  virtual void awaken();
  void run();
  bool treat_input(char input);
  virtual bool receive(const char* message, sid_t source);
  void receive_hello(sid_t source);
  virtual void queue(sid_t _next);
  virtual void print_info();
  // subroutines
  virtual void critical_section();
  virtual void liberation();
  virtual void send_token(sid_t destination);
  virtual void receive_token();
private:
  void supplication();
  void receive_request(sid_t source);
};

#endif // NAIMITREHELSITE_HPP_INCLUDED
