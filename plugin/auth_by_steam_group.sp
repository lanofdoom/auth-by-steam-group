#include <sourcemod>
#include "../extension/auth_by_steam_group.inc"

#define CVAR_MAX_LENGTH 255

Handle g_steam_group_id;
Handle g_steam_key;

public const Plugin myinfo = {
    name = "Authenticate by Steam Group Plugin", author = "LAN of DOOM",
    description = "Block non-members of a Steam group from joining",
    version = "1.0.0.0",
    url = "https://lanofdoom.github.io/auth-by-steam-group/"};

void KickUnauthorizedUser(int client_id) {
  KickClient(client_id, "You are not on the server's allow list.");
}

void OnJoin(Handle event, const char[] unused_name,
            bool unused_dont_broadcast) {
  char group_id[CVAR_MAX_LENGTH];
  GetConVarString(g_steam_group_id, group_id, CVAR_MAX_LENGTH);

  char steam_key[CVAR_MAX_LENGTH];
  GetConVarString(g_steam_key, steam_key, CVAR_MAX_LENGTH);

  int client_id = GetEventInt(event, "userid");

  char network_id[CVAR_MAX_LENGTH];
  GetEventString(event, "networkid", network_id, CVAR_MAX_LENGTH, "");

  if (!strlen(group_id) || !strlen(steam_key) || !strlen(network_id)) {
    return;
  }

  AuthBySteamGroup_CheckUser(client_id, network_id, group_id, steam_key,
                             KickUnauthorizedUser);
}

public void OnPluginStart() {
  g_steam_group_id =
      CreateConVar("sm_auth_by_steam_group_id", "",
                   "The ID of the steam group to allow on the server. If " ...
                   "empty, all players are allowed to join",
                   FCVAR_NOTIFY);
  g_steam_key = CreateConVar("sm_auth_by_steam_steam_key", "",
                             "The steam key to use for API lookups. If " ...
                             "empty or invalid, all players are allowed to " ...
                             "join.",
                             FCVAR_NOTIFY);
  HookEvent("player_connect", OnJoin);
}