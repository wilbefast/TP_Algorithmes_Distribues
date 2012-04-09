#ifndef NTDEMON_HPP_INCLUDED
#define NTDEMON_HPP_INCLUDED

#include "Demon.hpp"

// Naimi Trehel Demon

class NTDemon : public Demon
{
  /* METHODS */
public:
  // creation, destruction
  NTDemon(const char* registry_file);
};

#endif // NTDEMON_HPP_INCLUDED
