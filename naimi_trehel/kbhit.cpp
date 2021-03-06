#include "kbhit.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

int kbhit()
{
  int ch;
  int oldf;
  struct termios old_termios;
  struct termios new_termios;

  tcgetattr( STDIN_FILENO, &old_termios );
  new_termios = old_termios;
  new_termios.c_lflag &= ~( ICANON | ECHO );
  tcsetattr( STDIN_FILENO, TCSANOW, &new_termios );
  oldf = fcntl( STDIN_FILENO, F_GETFL, 0 );
  fcntl( STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK );

  ch = getchar();

  tcsetattr( STDIN_FILENO, TCSANOW, &old_termios );
  fcntl( STDIN_FILENO, F_SETFL, oldf );

  // return a character if one has been entered (0 is returned if not)
  if( ch == EOF )
    return 0;
  else
    return ch;
}

