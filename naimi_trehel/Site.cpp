// File streaming
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>     // for parsing messages

#include "Site.hpp"

#include "SDL_assert.h"
#include "kbhit.hpp"

#define REGISTRY "registry.txt"

#define BASE_PORT 49152
#define LOCALHOST_HEX 0x10007f
#define BASE_PORT_HEX 0xc0
#define PORT_SPACING_HEX 0x100
#define PACKET_SIZE 512

#define ID2PORT(id) BASE_PORT_HEX+id*PORT_SPACING_HEX
#define PORT2ID(port) (port-BASE_PORT_HEX)/PORT_SPACING_HEX

#define HELLO "hello"
#define KEY_QUIT 'q'
#define KEY_CLEAR 'c'
#define KEY_INFO 'i'

using namespace std;

/* CREATION & DESTRUCTION */

Site::Site() :
socket(),
packet(NULL),
this_tick(0),
next_tick(0),
message_data(),
id(0),
peers(),
state(ERROR),
logger(NULL)
{
  /* Create the logger */
  logger = new SiteLogger(this);

  /* Try to initialise the Site, catch errors */
  if(init() != EXIT_SUCCESS)
    WARN("Site::Site", "Initialisation failed");
  else
    state = ASLEEP;
}

int Site::init()
{
  /* Read and write in the registry */
  ASSERT(register_id() == EXIT_SUCCESS, "Reading/editing registry");

	/* Open a socket on the port corresponding to our identifier */
	ASSERT_NET(socket = SDLNet_UDP_Open(BASE_PORT+id), "Opening UDP socket");

	/* Allocate memory for the packet */
	ASSERT(packet = SDLNet_AllocPacket(PACKET_SIZE),
          "Allocating memory for packet");

  /* All clear! */
  return EXIT_SUCCESS;
}

int Site::register_id()
{
  /* Open in write mode first in order to block other from accessing the file */
  ofstream oregistry(REGISTRY, ios::app);
  ASSERT(oregistry, "Opening registry file-writer");

  /* Open in read mode */
  ifstream iregistry(REGISTRY);
  ASSERT(iregistry, "Opening registry file-reader");

  /* Read the identifiers of the other Sites */
  unsigned int new_peer;
  while(iregistry >> new_peer)
    peers.push_back(new_peer);

  /* choose an identifier not already taken by another site */
  for(id = 0; id < MAX_SITES; id++)
  {
    bool in_use = false;

    // check whether the current identifier is in use by a peer
    for(sid_list_it i = peers.begin(); i != peers.end(); i++)
      if((*i) == id)
      {
        // skip this identifier if anybody else is using it
        in_use = true;
        break;
      }

    // stop at the first identifier not used by any of the Site's peers
    if(!in_use)
      break;
  }

  /* Register this identifier in the file */
  oregistry << id << endl;
  logger->write("registered my identifier");

  /* All clear! */
  return EXIT_SUCCESS;

  // NB - all streams are closed automatically at the end of the block
}

void Site::awaken()
{
  /* Generic wake-up call which does nothing special */
  logger->write("woke up");
  state = IDLE;
}

Site::~Site()
{
  logger->write("goodbye cruel word!");

  /* Destroy the logger */
  delete logger;


  /* Attempt to clean up the mess */
  WARN_IF(shutdown() != EXIT_SUCCESS, "Site::~Site",
          "Site didn't exit cleanly!");
}

int Site::shutdown()
{
  /* Remove identifier from registry */
  ASSERT(unregister_id() == EXIT_SUCCESS, "Removing identifier from registry");

  /* Clean up SDL - "always succeeds" */
	SDLNet_FreePacket(packet);
	SDLNet_UDP_Close(socket);

	/* All clear! */
	return EXIT_SUCCESS;
}

int Site::unregister_id()
{
  /* Create a temporary copy of the register */
  ofstream otemp("temp.txt");
  ASSERT(otemp, "Opening 'temp.txt' file-writer");

  /* Open in read mode */
  ifstream iregistry(REGISTRY);
  ASSERT(iregistry, "Opening registry file-reader");

  /* Copy all identifiers except this Site's */
  sid_t peer;
  while(iregistry >> peer)
    if(peer != id)
      otemp << peer << endl;

  /* Out with the old, in with the new */
  remove(REGISTRY);
  rename("temp.txt", REGISTRY);

  /* All clear! */
  return EXIT_SUCCESS;
}


/* MAIN LOOP */

void Site::start()
{
  // Make sure them site has been correctly initialised
  if(state != ASLEEP)
    WARN_RTN_VOID("Site::start", "Not properly initialised");

  // Let other sites know that this one has joined the fray
  broadcast(HELLO);

  // Wake up method defined by specific algorithm
  awaken();

  // Run until there's a problem or a manual shut-down
  while(state != ERROR && state != SHUTDOWN)
    // Run method should be partially masked by specific algorithm
    run();
}

void Site::run()
{
  // Don't use 100% of the CPU !
  wait();

  // Check for key-presses
  char input = kbhit();
  // verify that the keyboard event was indeed consumed
  if(!treat_input(input))
    // report error if it remained unrecognised
    cout << "UNRECOGNISED input '" << input << "'!" << endl;

  // Check inbox
  if (SDLNet_UDP_Recv(socket, packet))
  {
    const char* message = (char*)packet->data;
    sid_t source = PORT2ID(packet->address.port);
    // verify that the communicate event was indeed consumed
    if(!receive(message, source))
      // report error if it remained unrecognised
      logger->write("Unknown message \"%s\" from %d", message, source);
  }
}

// Regulate the number of frames per second, pausing only if need be
void Site::wait()
{
  // Get the current time-stamp
  this_tick = SDL_GetTicks();

  // If it's not yet time for the next update, wait a will
	if (this_tick < next_tick )
		SDL_Delay(next_tick - this_tick);

  // Calculate when the next update should be
	next_tick = this_tick + (1000/MAX_HERTZ);
}

void Site::parse_data(string s)
{
  // recursion base case
  if(s.length() < 1)
    return;

  // add the next predecessor
  size_t comma = s.find(',');
  message_data.push_back(atoi(s.substr(0, comma).c_str()));

  // recursive call for string processing, LISP style !
  parse_data(s.substr(comma+1));
}

/* QUERY */

sid_t Site::getId() const
{
  return id;
}

/* USER INTERFACE */

bool Site::treat_input(char input)
{
  /* Generic interpret input method only deals with quit command */

  // ignore all null inputs
  if(!input)
    return true;
  else
    cout << " -> ";

  // switch on remaining inputs
  switch(input)
  {
    case KEY_QUIT:
      cout << "QUIT" << endl;
      state = SHUTDOWN;
      // event consumed
      return true;
    break;

    case KEY_INFO:
      cout << "INFORMATION" << endl;
      print_info();
      // event consumed
      return true;

    case KEY_CLEAR:
      cout << string(30, '\n');
      // event consumed
      return true;

    default:
      // event not consumed
      return false;
    break;
  }
}

void Site::print_info()
{
  // Site identifier
  cout << "id = " << id << endl;

  // Site peers
  cout << "peers = [ ";
  for(sid_list_it i = peers.begin(); i != peers.end(); i++)
    cout << (*i) << " ";
  cout << "]" << endl;

  // Site state
  cout << "state = " << state << endl;
}

/* COMMUNICATION */

void Site::send_data(const char* header, sid_t destination, int argc, ...)
{
  // transform '...' into a va_list structure
  va_list arguments;
  va_start(arguments, argc);
  send_data(header, destination, argc, arguments);

}

void Site::send_data(const char* header, sid_t destination, int argc,
                    va_list arguments)
{
  // create a string-builder object, place the header in it
  stringstream builder;
  {
    string temp(header);
    builder << temp << ':';
  }

  // concatenate all number to the end of the header
  for(int i = 0; i < argc; i++)
    builder << va_arg(arguments, int) << ',';
  va_end(arguments);

  // send the result
  send(builder.str().c_str(), destination);
}

void Site::send(const char* message, sid_t destination)
{
  /* Build packet */
  unsigned int length = strlen(message) + 1;
  memcpy ((char*)packet->data, message, length);
  packet->address.host = LOCALHOST_HEX;
  packet->address.port = ID2PORT(destination);
  packet->len = length;

  /* Send packet to destination */
  SDLNet_UDP_Send(socket, -1, packet);
  logger->write("sent \"%s\" to Site %d", message, destination);
}

void Site::broadcast(const char* message)
{
  // Send message to each peer
  for(sid_list_it i = peers.begin(); i != peers.end(); i++)
    send(message, (*i));
}

void Site::broadcast_data(const char* header, int argc, ...)
{
  // Transform '...' into a va_list structure
  va_list arguments;
  va_start(arguments, argc);

  // Send data to each peer
  for(sid_list_it i = peers.begin(); i != peers.end(); i++)
    send_data(header, (*i), argc, arguments);
}

bool Site::receive(const char* message, sid_t source)
{
  logger->write("received \"%s\" from Site %d", message, source);

  // Create a string object for easy manipulation
  string s_message(message);

  // Parse any arguments which may be contained in the message
  message_data.clear();
  size_t colon = s_message.find(':');
  if(colon != string::npos)
    parse_data(s_message.substr(colon+1));

  // Site registration
  if(!s_message.compare(HELLO))
  {
    // call specific code of algorithm this Site uses
    receive_hello(source);
    return true;  // consume event
  }

  // Event has not been consumed
  return false;
}

void Site::receive_hello(sid_t source)
{
  // add this new Site to the list of known peers
  peers.push_back(source);
  logger->write("added %d as a new peer", source);
}
