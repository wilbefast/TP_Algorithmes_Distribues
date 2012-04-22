#include "NTDemonX.hpp"

#include "wjd_math.hpp"

NTDemonX::NTDemonX() :
Demon(),
has_token(false),
is_requesting(false),
father(-1),
next(-1)
{
}

int NTDemonX::awaken()
{
  state = NORMAL;

  // If this is the first Demon created then it has the token
  if(peers.size() == 0)
  {
    has_token = true;
    printf("Demon %d: 'I have the token'\n", id);
  }

  // Otherwise we need to let the other know of this new Demon
  else
  {
    father = peers.front();
    printf("Demon %d: 'Demon %d has the token'\n", id, father);
    broadcast("new");
  }

  /* All clear ! */
  return EXIT_SUCCESS;
}

void NTDemonX::receive(const char* message, id_t source)
{
  // A new Demon is registring its existence
  if(!strcmp(message, "new"))
  {
    peers.push_back(source);
    printf("Demon %d: 'I added %d as a new peer'\n", id, source);
  }
}
