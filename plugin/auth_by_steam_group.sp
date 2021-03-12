#include <sourcemod>

#include "../extension/auth_by_steam_group.inc"

#define CVAR_MAX_LENGTH 256

Handle g_allow_join_enabled;
Handle g_steam_group_id;
Handle g_steam_key;

public const Plugin myinfo = {
    name = "Authenticate by Steam Group Plugin", author = "LAN of DOOM",
    description = "Block non-members of a Steam group from joining",
    version = "2.0.2",
    url = "https://lanofdoom.github.io/auth-by-steam-group/"};

public Extension __ext_auth_by_steam_group = {
  name = "Authenticate by Steam Group Extension",
  file = "auth_by_steam_group.ext",
  autoload = 1,
  required = 1};

public void OnClientAuthorized(int client, const char[] auth) {
  char group_id[CVAR_MAX_LENGTH];
  GetConVarString(g_steam_group_id, group_id, CVAR_MAX_LENGTH);
  if (!strlen(group_id)) {
    return;
  }

  char steam_key[CVAR_MAX_LENGTH];
  GetConVarString(g_steam_key, steam_key, CVAR_MAX_LENGTH);
  if (!strlen(steam_key)) {
    return;
  }

  AuthBySteamGroup_CheckUser(client, group_id, steam_key);
}

public void OnClientSayCommand_Post(int client, const char[] command,
                                    const char[] text) {
  char group_id[CVAR_MAX_LENGTH];
  GetConVarString(g_steam_group_id, group_id, CVAR_MAX_LENGTH);
  if (!strlen(group_id)) {
    return;
  }

  char steam_key[CVAR_MAX_LENGTH];
  GetConVarString(g_steam_key, steam_key, CVAR_MAX_LENGTH);
  if (!strlen(steam_key)) {
    return;
  }

  if (!GetConVarBool(g_allow_join_enabled)) {
    return;
  }

  if (StrEqual(text, "allowjoin")) {
    AuthBySteamGroup_AllowJoin(client, group_id, steam_key);
    return;
  }

  if (StrEqual(text, "kicklist")) {
    AuthBySteamGroup_PrintKickList(client, group_id, steam_key);
    return;
  }

  if (AuthBySteamGroup_TryProcessKickCommand(client, group_id, steam_key, text)) {
    return;
  }
}

public void OnGameFrame() {
  AuthBySteamGroup_OnFrame();
}

public void OnMapStart() {
  AuthBySteamGroup_RotateAccessGrants();
}

public void OnPluginStart() {
  g_allow_join_enabled =
      CreateConVar("sm_auth_by_steam_allowjoin_enabled", "1",
                   "If true, members of the authorized group can say " ...
                   "'allowjoin' in chat to allow the next unauthorized " ...
                   "player that attempts to join the server.");
  g_steam_group_id =
      CreateConVar("sm_auth_by_steam_group_group_id", "",
                   "A comma-separated list of IDs of the Steam groups " ...
                   "allowed to join the server. If empty, all players are " ...
                   "allowed to join.");
  g_steam_key = CreateConVar("sm_auth_by_steam_group_steam_key", "",
                             "The Steam key to use for API lookups. If " ...
                             "empty or invalid, all players are allowed to " ...
                             "join.");
}