#include <sourcemod>

#include "adt_array.inc"
#include "../extension/auth_by_steam_group.inc"

#define CVAR_MAX_LENGTH 256

ArrayList g_this_round_allowed_clients;
ArrayList g_last_round_allowed_clients;
ArrayList g_kickable_player_client_indices;
ArrayList g_kickable_player_commands;
ArrayList g_group_members;
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

  if (g_group_members.FindString(steam_id) != -1) {
    return true;
  }

  if (AuthBySteamGroup_CheckUser(client, steam_id, group_id, steam_key)) {
    g_group_members.PushString(steam_id);
    return true;
  }

  return false;
}

bool CheckUserAllowedAccess(int client) {
  char steam_id[CVAR_MAX_LENGTH];
  if (!GetClientAuthId(client, AuthId_SteamID64, steam_id, CVAR_MAX_LENGTH)) {
    return true;
  }

  if (g_this_round_allowed_clients.FindString(steam_id) != -1) {
    return true;
  }

  char kick_command[CVAR_MAX_LENGTH];
  Format(kick_command, CVAR_MAX_LENGTH, "kickplayer %s", steam_id);

  if (g_last_round_allowed_clients.FindString(steam_id) != -1) {
    g_this_round_allowed_clients.PushString(steam_id);
    g_kickable_player_commands.PushString(kick_command);
    g_kickable_player_client_indices.Push(client);
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
      PrintToChatAll("An unauthorized player was granted access");
    }

    PrintToChatAll("If this was not the player you intended to let in, use " ...
                   "command '%s' to remove them from the server or use " ...
                   "command 'kicklist' to view the list of all kickable " ...
                   "player.",
                   kick_command);

    g_this_round_allowed_clients.PushString(steam_id);
    g_kickable_player_commands.PushString(kick_command);
    g_kickable_player_client_indices.Push(client);
    g_allow_next_access = false;
  }

  return true;
}

void PrintKickList(int client) {
  if (g_kickable_player_commands.Length == 0) {
    PrintToChat(client, "No kickable players.");
    return;
  }

  for (int i = 0; i < g_kickable_player_commands.Length; i++) {
    char kick_command[CVAR_MAX_LENGTH];
    g_kickable_player_commands.GetString(i, kick_command, CVAR_MAX_LENGTH);

    char client_name[MAX_NAME_LENGTH];
    if (!GetClientName(client, client_name, MAX_NAME_LENGTH)) {
      PrintToChat(client, kick_command);
    } else {
      PrintToChat(client, "%s: %s", kick_command, client_name);
    }
  }
}

int FindKickablePlayerIndex(const char[] text) {
  for (int i = 0; i < g_kickable_player_commands.Length; i++) {
    char kick_command[CVAR_MAX_LENGTH];
    g_kickable_player_commands.GetString(i, kick_command, CVAR_MAX_LENGTH);

    if (StrEqual(kick_command, text)) {
      return i;
    }
  }

  return -1;
}

public void OnClientAuthorized(int client, const char[] auth) {
  if (!CheckUserAllowedAccess(client)) {
    KickClient(client, "You are not on the server's allowlist");
  }
}

public void OnClientDisconnect(int client) {
  int index = g_kickable_player_client_indices.FindValue(client);
  if (index != -1) {
    g_kickable_player_client_indices.Erase(index);
    g_kickable_player_commands.Erase(index);
  }
}

public void OnClientSayCommand_Post(int client, const char[] command,
                                    const char[] text) {
  if (!GetConVarBool(g_allow_access_enabled)) {
    return;
  }

  int kickable_player_index = FindKickablePlayerIndex(text);

  if (!StrEqual(text, "allowaccess") &&
      !StrEqual(text, "kicklist") &&
      kickable_player_index == -1) {
    return;
  }

  char steam_id[CVAR_MAX_LENGTH];
  if (!GetClientAuthId(client, AuthId_SteamID64, steam_id, CVAR_MAX_LENGTH) ||
      g_group_members.FindString(steam_id) == -1) {
    PrintToChat(client, "You are not authorized to use that command.");
    return;
  }

  if (StrEqual(text, "allowaccess")) {
    PrintToChatAll("The next unauthorized player that attempts to join " ...
                   "will be allowed access.");
    g_allow_next_access = true;
  } else if (StrEqual(text, "kicklist")) {
    PrintKickList(client);
  } else if (kickable_player_index != -1) {
    int to_kick = g_kickable_player_client_indices.Get(kickable_player_index);
    char kicked_player_name[MAX_NAME_LENGTH];
    if (GetClientName(to_kick, kicked_player_name, MAX_NAME_LENGTH)) {
      PrintToChatAll("%s was kicked", kicked_player_name);
    } else {
      PrintToChatAll("Player kicked");
    }

    char ks_id[CVAR_MAX_LENGTH];
    if (GetClientAuthId(to_kick, AuthId_SteamID64, ks_id, CVAR_MAX_LENGTH)) {
      int delete_index = g_this_round_allowed_clients.FindString(ks_id);
      if (delete_index != -1) {
        g_this_round_allowed_clients.Erase(delete_index);
      }
      delete_index = g_last_round_allowed_clients.FindString(ks_id);
      if (delete_index != -1) {
        g_last_round_allowed_clients.Erase(delete_index);
      }
    }

    g_kickable_player_client_indices.Erase(kickable_player_index);
    g_kickable_player_commands.Erase(kickable_player_index);

    KickClient(to_kick, "You have been kicked from the server");
  }
}

public void OnMapStart() {
  g_last_round_allowed_clients.Clear();
  for (int i = 0; i < g_this_round_allowed_clients.Length; i++) {
    char steam_id[CVAR_MAX_LENGTH];
    g_this_round_allowed_clients.GetString(i, steam_id, CVAR_MAX_LENGTH);
    g_last_round_allowed_clients.PushString(steam_id);
  }
  g_this_round_allowed_clients.Clear();
  g_kickable_player_client_indices.Clear();
  g_kickable_player_commands.Clear();
  g_group_members.Clear();
  g_allow_next_access = false;
}

public void OnPluginStart() {
  g_this_round_allowed_clients = new ArrayList(CVAR_MAX_LENGTH);
  g_last_round_allowed_clients = new ArrayList(CVAR_MAX_LENGTH);
  g_kickable_player_commands = new ArrayList(CVAR_MAX_LENGTH);
  g_group_members = new ArrayList(CVAR_MAX_LENGTH);
  g_kickable_player_client_indices = new ArrayList(1);
  g_allow_access_enabled =
      CreateConVar("sm_auth_by_steam_allowaccess_enabled", "",
                   "If true, members of the authorized group can say " ...
                   "'allowaccess' in chat to grant acccess to the next " ...
                   "unauthorized player that attempts to join the server.");
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