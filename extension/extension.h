#ifndef _AUTH_BY_STEAM_GROUP_EXTENSION_EXTENSION_
#define _AUTH_BY_STEAM_GROUP_EXTENSION_EXTENSION_

#include "smsdk_ext.h"

class AuthBySteamGroup : public SDKExtension {
public:
  virtual bool SDK_OnLoad(char *error, size_t maxlen, bool late);
  virtual void SDK_OnAllLoaded();
  virtual bool QueryRunning(char *error, size_t maxlength);
};

#endif // _AUTH_BY_STEAM_GROUP_EXTENSION_EXTENSION_
