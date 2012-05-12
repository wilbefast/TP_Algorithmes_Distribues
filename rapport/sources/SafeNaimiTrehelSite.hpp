class SafeNaimiTrehelSite : public NaimiTrehelSite
{
  /* ATTRIBUTES */
private:
    // predecessors mapped to their distance from the token-holder
	sid_map_t predecessors;
	// time till next check
	int check_timer;
	// time till reply is assumed not to be coming
	int reply_timer;
	// this site's position in the queue
	int queue_position;

  /* METHODS */
protected:
	// called when the check timer runs out
	void poll();
	// called if currently polling predecessors with "are you alive"
	void poll_predecessors();
	// called when the reply timer runs out
	void timeout();
private:
	// called if currently polling predecessors with "are you alive"
	void timeout_predecessors();
	// called if currently reconnecting with the rest of the queue
	void timeout_reconnect();
	// called if currently searching for the queue
	void timeout_search_queue();
};
