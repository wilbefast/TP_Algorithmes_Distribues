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
  id_t father;
  id_t next;

  /* METHODS */
public:
  // creation, destruction
  NTDemonX();
protected:
  // overrides
  int awaken();
  void receive(const char* message, id_t source);
};

#endif // NTDEMONX_HPP_INCLUDED
