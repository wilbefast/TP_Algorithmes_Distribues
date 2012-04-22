#ifndef NTDEMONX_HPP_INCLUDED
#define NTDEMONX_HPP_INCLUDED

#include "Demon.hpp"

// Extended Naimi Trehel Demon

class NTDemonX : public Demon
{
  /* ATTRIBUTES */
private:
  bool has_token;
  bool is_requesting;
  sid_t father;
  sid_t next;

  /* METHODS */
public:
  // creation, destruction
  NTDemonX();
protected:
  // overrides
  void awaken();
  void idle();
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

#endif // NTDEMONX_HPP_INCLUDED
