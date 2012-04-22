#include "wjd_math.hpp"

int decimals(int n)
{
  int counter = 1;
  while(n /= 10)
    counter++;

  return counter;
}
