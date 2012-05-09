#ifndef SITELOGGER_HPP_INCLUDED
#define SITELOGGER_HPP_INCLUDED

// mutual dependence
class SiteLogger;
#include "Site.hpp"

class SiteLogger
{
  /* ATTRIBUTES */
private:
  Site* owner;

  /* METHODS */
public:
  // creation, destruction
  SiteLogger(Site* _owner);
};

#endif // SITELOGGER_HPP_INCLUDED
