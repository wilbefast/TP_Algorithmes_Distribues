/*
Assertions/warnings for Android native development using SDL
William J. Dyce
23 July 2011
*/


#ifndef ASSERT_HPP_INCLUDED
#define ASSERT_HPP_INCLUDED

/* ERROR LOGGING */

#define LOG(level, what, why);                          \
		printf("%s - %s: %s\n", level, what, why);
#define LOG_I(what, why);                               \
    LOG("INFO", what, why);
#define LOG_W(what, why);                               \
    LOG("WARNING", what, why);
#define LOG_E(what, why);                               \
    LOG("ERROR", what, why);

/* WARNINGS */

#define WARN(what, why);                                \
        LOG_W(what, why);

#define WARN_IF(problem, what, why);                    \
    if(problem)                                         \
        WARN(what, why);

#define WARN_RTN(what, why, rtn);                       \
  {                                                     \
      WARN(what, why);                                  \
      return(rtn);                                      \
  }

/* ASSERTIONS */

#define ASSERT_AUX_RTN(assertion, what, why, rtn);      \
	if(!(assertion))                                      \
	{									                                    \
		LOG_E(what, why);	                                  \
		return rtn;                                         \
  }                                                     \
  else                                                  \
    LOG_I(what, "Okay");

#define ASSERT_AUX(assertion, what, why);               \
	ASSERT_AUX_RTN(assertion, what, why, EXIT_FAILURE);

#define ASSERT_RTN(assertion, what, rtn);               \
  ASSERT_AUX_RTN(assertion, what, "Failed", rtn);

#define ASSERT(assertion, what);                        \
  ASSERT_AUX(assertion, what, "Failed");

#define ASSERT_SDL(assertion, what);                    \
  ASSERT_AUX(assertion, what, SDL_GetError());

#define ASSERT_GL(assertion, what);                     \
  ASSERT_AUX(assertion, what, GL_GetError());

#define ASSERT_MIX(assertion, what);                    \
    ASSERT_AUX(assertion, what, Mix_GetError());

#define ASSERT_NET(assertion, what);                    \
    ASSERT_AUX(assertion, what, SDLNet_GetError());

#endif // ASSERT_HPP_INCLUDED
