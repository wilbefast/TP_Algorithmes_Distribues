class NaimiTrehelSite : public Site
{
  /* ATTRIBUTES */
  private:
	// does this site have the token ?
	bool has_token;
  protected:
	// the site's father in the tree
	sid_t father;
	// the site's successor in the queue
	sid_t next;

  /* METHODS */
  private:
	// request the critical section
	void supplication();
  protected:
	// enter the critical section
	virtual void critical_section();
	// exit the critical section
	virtual void liberation();
	// send the token to a site identified by a number
	virtual void send_token(sid_t destination);
	// called internally when token is received
	virtual void receive_token();
};
