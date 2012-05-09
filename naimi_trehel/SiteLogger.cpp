#include "SiteLogger.hpp"

#include <time.h>

using namespace std;

/* CREATION, DESTRUCTION */

SiteLogger::SiteLogger(Site* _owner) :
owner(_owner)
{
}


/* READ, WRITE */

void SiteLogger::write(const char* format, ...)
{
  static struct tm* current;
	static time_t now;
	time(&now);
	current = localtime(&now);

  // add headers depending on the time and Site id
  printf("%i:%i:%i - Site %d: '",
    current->tm_hour, current->tm_min, current->tm_sec, owner->getId());

  // print the rest of the message
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);

  // close off message
  printf("'\n");
}
