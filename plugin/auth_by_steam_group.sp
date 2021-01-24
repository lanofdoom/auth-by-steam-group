#include <sourcemod>

#include "adt_array.inc"
#include "../extension/auth_by_steam_group.inc"

#define CVAR_MAX_LENGTH 256

ArrayList g_allowed_clients;
bool g_allow_next_access = false;
Handle g_allow_access_enabled;
Handle g_steam_group_id;
Handle g_steam_key;

public const Plugin myinfo = {
    name = "Authenticate by Steam Group Plugin", author = "LAN of DOOM",
    description = "Block non-members of a Steam group from joining",
    version = "1.2.1",
    url = "https://lanofdoom.github.io/auth-by-steam-group/"};

public Extension __ext_auth_by_steam_group = {
  name = "Authenticate by Steam Group Extension",
  file = "auth_by_steam_group.ext",
  autoload = 1,
  required = 1};

bool CheckUserInGroup(int client) {
  char group_id[CVAR_MAX_LENGTH];
  GetConVarString(g_steam_group_id, group_id, CVAR_MAX_LENGTH);
  if (!strlen(group_id)) {
    return true;
  }

  char steam_key[CVAR_MAX_LENGTH];
  GetConVarString(g_steam_key, steam_key, CVAR_MAX_LENGTH);
  if (!strlen(steam_key)) {
    return true;
  }

  char steam_id[CVAR_MAX_LENGTH];
  if (!GetClientAuthId(client, AuthId_SteamID64, steam_id, CVAR_MAX_LENGTH)) {
    return true;
  }

  return AuthBySteamGroup_CheckUser(client, steam_id, group_id, steam_key);
}

bool CheckUserAllowedAccess(int client) {
  char steam_id[CVAR_MAX_LENGTH];
  if (!GetClientAuthId(client, AuthId_SteamID64, steam_id, CVAR_MAX_LENGTH)) {
    return true;
  }

  if (g_allowed_clients.FindString(steam_id) != -1) {
    return true;
  }

  if (!CheckUserInGroup(client) && !g_allow_next_access) {
    return false;
  }

  if (g_allow_next_access) {
    char client_name[MAX_NAME_LENGTH];
    if (GetClientName(client, client_name, MAX_NAME_LENGTH)) {
      PrintToChatAll("%s was granted access", client_name);
    } else {
      PrintToChatAll("An unauthorized user was granted access");
    }

    g_allowed_clients.PushString(steam_id);
    g_allow_next_access = false;
  }

  return true;
}

public void OnClientAuthorized(int client, const char[] auth) {
  if (!CheckUserAllowedAccess(client)) {
    KickClient(client, "You are not on the server's allowlist");
  }
}

public void OnClientSayCommand_Post(int client, const char[] command,
                                    const char[] text) {
  if (!GetConVarBool(g_allow_access_enabled) ||
      !StrEqual(text, "allowaccess")) {
    return;
  }

  if (!CheckUserInGroup(client)) {
    PrintToChat(client, "You are not authorized to use that command");
    return;
  }

  PrintToChatAll("The next unauthorized user that attempts to join will be " ...
                 "allowed access");
  g_allow_next_access = true;
}

public void OnMapEnd() {
  g_allowed_clients.Clear();
  for (int client = 1; client <= MaxClients; client++) {
    if (IsClientInGame(client)) {
      char auth[CVAR_MAX_LENGTH];
      if (!GetClientAuthId(client, AuthId_SteamID64, auth, CVAR_MAX_LENGTH)) {
        PrintToServer("GetClientAuthId failed for client %d", client);
        continue;
      }
      g_allowed_clients.PushString(auth);
    }
  }
}

public void OnPluginStart() {
  g_allow_access_enabled =
      CreateConVar("sm_auth_by_steam_allowaccess_enabled", "",
                   "If true, members of the authorized group can say " ...
                   "'allowaccess' in chat to grant acccess to the next " ...
                   "unauthorized user that attempts to join the server.",
                   FCVAR_NOTIFY);
  g_steam_group_id =
      CreateConVar("sm_auth_by_steam_group_group_id", "",
                   "A comma-separated list of IDs of the Steam groups " ...
                   "allowed to join the server. If empty, all players are " ...
                   "allowed to join.",
                   FCVAR_NOTIFY);
  g_steam_key = CreateConVar("sm_auth_by_steam_group_steam_key", "",
                             "The Steam key to use for API lookups. If " ...
                             "empty or invalid, all players are allowed to " ...
                             "join.",
                             FCVAR_NOTIFY);
}