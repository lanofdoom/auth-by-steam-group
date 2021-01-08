#ifndef _AUTH_BY_STEAM_GROUP_EXTENSION_EXTENSION_
#define _AUTH_BY_STEAM_GROUP_EXTENSION_EXTENSION_

#include "smsdk_ext.h"

class AuthBySteamGroup : public SDKExtension {
public:
  virtual void SDK_OnAllLoaded();
};

#endif // _AUTH_BY_STEAM_GROUP_EXTENSION_EXTENSION_
