#ifndef _AUTH_BY_STEAM_GROUP_EXTENSION_SMSDK_CONFIG_
#define _AUTH_BY_STEAM_GROUP_EXTENSION_SMSDK_CONFIG_

#define SMEXT_CONF_NAME         "Authenticate by Steam Group Extension"
#define SMEXT_CONF_DESCRIPTION  "Block non-members of a Steam group from joining"
#define SMEXT_CONF_VERSION      "2.2.0"
#define SMEXT_CONF_AUTHOR       "LAN of DOOM"
#define SMEXT_CONF_URL          "https://lanofdoom.github.io/auth-by-steam-group/"
#define SMEXT_CONF_LOGTAG       "LODAUTH"
#define SMEXT_CONF_LICENSE      "GPL"
#define SMEXT_CONF_DATESTRING   __DATE__

#define SMEXT_LINK(name) SDKExtension *g_pExtensionIface = name;

// Enable Required SourceMod Interfaces
#define SMEXT_ENABLE_GAMEHELPERS
#define SMEXT_ENABLE_PLAYERHELPERS

#endif // _AUTH_BY_STEAM_GROUP_EXTENSION_SMSDK_CONFIG_
