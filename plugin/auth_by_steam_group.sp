#include <sourcemod>
#include "../extension/auth_by_steam_group.inc"

#define CVAR_MAX_LENGTH 255

Handle g_steam_group_id;
Handle g_steam_key;

public const Plugin myinfo = {
    name = "Authenticate by Steam Group Plugin", author = "LAN of DOOM",
    description = "Block non-members of a Steam group from joining",
    version = "1.0.0",
    url = "https://lanofdoom.github.io/auth-by-steam-group/"};

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

  char steam_id[CVAR_MAX_LENGTH];
  if (!GetClientAuthId(client, AuthId_SteamID64, steam_id, CVAR_MAX_LENGTH)) {
    return;
  }

  if (!AuthBySteamGroup_CheckUser(client, steam_id, group_id, steam_key)) {
    KickClient(client, "You are not on the server's allow list.");
  }
}

public void OnPluginStart() {
  g_steam_group_id =
      CreateConVar("sm_auth_by_steam_group_group_id", "",
                   "The ID of the steam group to allow on the server. If " ...
                   "empty, all players are allowed to join",
                   FCVAR_NOTIFY);
  g_steam_key = CreateConVar("sm_auth_by_steam_group_steam_key", "",
                             "The steam key to use for API lookups. If " ...
                             "empty or invalid, all players are allowed to " ...
                             "join.",
                             FCVAR_NOTIFY);
}