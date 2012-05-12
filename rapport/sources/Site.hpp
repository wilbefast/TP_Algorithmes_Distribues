class Site
{
  /* ATTRIBUTES */
  private:
	// SDL socket structure used for communication with other sites
	UDPsocket socket;
	// same memory is used for all packets sent and received by the site
	UDPpacket* packet;
  protected:
	// this site's identifier
	sid_t id;
	// a list of identifiers of known peers
	sid_list_t peers;

  /* METHODS */
  protected:
	// send a string to a site identifier by a number
	void send(const char* message, sid_t destination);
	// send a header plus sequence of integers
	void send_data(const char* header, sid_t destination, int argc, ...);
	// send a string to all known peers
	void broadcast(const char* message);
	// send a header plus sequence of integers to all known peers
	void broadcast_data(const char* header, int argc, ...);
	// method called iternally when message is received
	virtual bool receive(const char* message, sid_t source);
};
