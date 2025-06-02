#include "extension.h"

#ifndef _WIN32
#include <cstdlib>
#else
#include <winhttp.h>
#endif  // _WIN32

#include <algorithm>
#include <memory>
#include <string>
#include <utility>

namespace {

AuthBySteamGroup g_auth_by_steam_group;

#ifndef _WIN32

std::unique_ptr<std::string> DoHttpRequest(const std::string& server,
                                           const std::string& path) {
  std::string command = "curl -s https://" + server + path;

  FILE* pipe = popen(command.c_str(), "r");
  if (!pipe) {
    return nullptr;
  }

  std::string result;
  for (int c = fgetc(pipe); c = fgetc(pipe); c != EOF) {
    result += static_cast<char>(c);
  }

  fclose(pipe);

  return std::make_unique<std::string>(std::move(result));
}

#else

std::unique_ptr<std::string> DoHttpRequest(const std::string& server,
                                           const std::string& path) {
  HINTERNET session =
      WinHttpOpen(L"WinHTTP/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                  WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
  if (!session) {
    return std::unique_ptr<std::string>();
  }

  std::wstring wserver(server.begin(), server.end());
  HINTERNET connect =
      WinHttpConnect(session, wserver.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
  if (!connect) {
    WinHttpCloseHandle(session);
    return std::unique_ptr<std::string>();
  }

  std::wstring wpath(path.begin(), path.end());
  HINTERNET request = WinHttpOpenRequest(
      connect, L"GET", wpath.c_str(), NULL, WINHTTP_NO_REFERER,
      WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
  if (!request) {
    WinHttpCloseHandle(connect);
    WinHttpCloseHandle(session);
    return std::unique_ptr<std::string>();
  }

  BOOL results = WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                    WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
  if (!results) {
    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connect);
    WinHttpCloseHandle(session);
    return std::unique_ptr<std::string>();
  }

  results = WinHttpReceiveResponse(request, NULL);
  if (!results) {
    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connect);
    WinHttpCloseHandle(session);
    return std::unique_ptr<std::string>();
  }

  std::unique_ptr<std::string> data = std::make_unique<std::string>();
  for (;;) {
    DWORD size;
    if (!WinHttpQueryDataAvailable(request, &size)) {
      WinHttpCloseHandle(request);
      WinHttpCloseHandle(connect);
      WinHttpCloseHandle(session);
      return std::unique_ptr<std::string>();
    }

    std::string buffer(size, '\0');

    DWORD downloaded;
    if (!WinHttpReadData(request, (LPVOID)&buffer[0], size, &downloaded)) {
      WinHttpCloseHandle(request);
      WinHttpCloseHandle(connect);
      WinHttpCloseHandle(session);
      return std::unique_ptr<std::string>();
    }

    *data += buffer;

    if (size == 0) {
      break;
    }
  }

  WinHttpCloseHandle(request);
  WinHttpCloseHandle(connect);
  WinHttpCloseHandle(session);

  return data;
}

#endif  // _WIN32

std::vector<std::string> Tokenize(const std::string& value,
                                  const std::string& delims) {
  std::vector<std::string> tokens;

  bool new_token = true;
  for (char c : value) {
    if (delims.find(c) != std::string::npos) {
      new_token = true;
    } else {
      if (new_token) {
        tokens.push_back(std::string());
      }
      tokens.back().push_back(c);
      new_token = false;
    }
  }

  return tokens;
}

// This is a bit of a hack. Instead of properly parsing out the JSON, just
// break it up into tokens and iterate through them. This should be fine
// given the simplicity of the response, but this is worth fixing at some
// point.
std::set<std::string> ParseGroupIds(const std::string& response) {
  std::vector<std::string> tokens = Tokenize(response, "{}[]:\",");

  std::set<std::string> result;
  bool next_is_gid = false;
  for (const std::string& token : tokens) {
    if (token == "gid") {
      next_is_gid = true;
    } else {
      if (next_is_gid) {
        result.insert(token);
      }
      next_is_gid = false;
    }
  }

  return result;
}

std::pair<std::string, std::string> BuildSteamworksQueryUrl(
    uint64_t steam_id64, const std::string& steam_key) {
  std::string path = "/ISteamUser/GetUserGroupList/v1/?steamid=";
  path += std::to_string(steam_id64);
  path += "&key=";
  path += steam_key;
  return std::make_pair("api.steampowered.com", path);
}

std::pair<std::string, std::string> BuildCommunityQueryUrl(
    const std::string& steam_group_id) {
  std::string path = "/gid/[G:1:";
  path += steam_group_id;
  path += "]/memberslistxml/?xml=1";
  return std::make_pair("steamcommunity.com", path);
}

bool CheckGroupMembershipSteamworks(uint64_t steam_id64,
                                    const std::string& steam_group_id,
                                    const std::string& steam_api_key) {
  auto request = BuildSteamworksQueryUrl(steam_id64, steam_api_key);
  std::unique_ptr<std::string> response =
      DoHttpRequest(request.first, request.second);
  if (!response) {
    return false;
  }
  std::set<std::string> client_group_ids = ParseGroupIds(*response);
  return client_group_ids.count(steam_group_id) != 0;
}

bool CheckGroupMembershipCommunity(uint64_t steam_id64,
                                   const std::string& steam_group_id) {
  auto request = BuildCommunityQueryUrl(steam_group_id);
  std::unique_ptr<std::string> response =
      DoHttpRequest(request.first, request.second);
  if (!response) {
    return false;
  }
  return response->find(std::to_string(steam_id64)) != std::string::npos;
}

cell_t AllowJoin(IPluginContext* context, const cell_t* params) {
  int client_id = params[1];

  char* group_id;
  context->LocalToString(params[2], &group_id);

  char* steam_key;
  context->LocalToString(params[3], &steam_key);

  g_auth_by_steam_group.AllowJoin(client_id, group_id, steam_key);
  return 0;
}

cell_t DenyJoin(IPluginContext* context, const cell_t* params) {
  int client_id = params[1];

  char* group_id;
  context->LocalToString(params[2], &group_id);

  char* steam_key;
  context->LocalToString(params[3], &steam_key);

  g_auth_by_steam_group.DenyJoin(client_id, group_id, steam_key);
  return 0;
}

cell_t CheckUser(IPluginContext* context, const cell_t* params) {
  int client_id = params[1];

  char* group_id;
  context->LocalToString(params[2], &group_id);

  char* steam_key;
  context->LocalToString(params[3], &steam_key);

  g_auth_by_steam_group.CheckAccess(client_id, group_id, steam_key);
  return 0;
}

cell_t OnFrame(IPluginContext* context, const cell_t* params) {
  g_auth_by_steam_group.OnFrame();
  return 0;
}

cell_t PrintKickList(IPluginContext* context, const cell_t* params) {
  int client_id = params[1];

  char* group_id;
  context->LocalToString(params[2], &group_id);

  char* steam_key;
  context->LocalToString(params[3], &steam_key);

  g_auth_by_steam_group.PrintKickList(client_id, group_id, steam_key);
  return 0;
}

cell_t RotateAccessGrants(IPluginContext* context, const cell_t* params) {
  g_auth_by_steam_group.RotateAccessGrants();
  return 0;
}

cell_t TryProcessKickCommand(IPluginContext* context, const cell_t* params) {
  int client_id = params[1];

  char* group_id;
  context->LocalToString(params[2], &group_id);

  char* steam_key;
  context->LocalToString(params[3], &steam_key);

  char* text;
  context->LocalToString(params[4], &text);

  return g_auth_by_steam_group.TryProcessKickCommand(client_id, group_id,
                                                     steam_key, text);
}

static const sp_nativeinfo_t g_natives[] = {
    {"AuthBySteamGroup_AllowJoin", AllowJoin},
    {"AuthBySteamGroup_DenyJoin", DenyJoin},
    {"AuthBySteamGroup_CheckUser", CheckUser},
    {"AuthBySteamGroup_PrintKickList", PrintKickList},
    {"AuthBySteamGroup_OnFrame", OnFrame},
    {"AuthBySteamGroup_RotateAccessGrants", RotateAccessGrants},
    {"AuthBySteamGroup_TryProcessKickCommand", TryProcessKickCommand},
    {nullptr, nullptr},
};

}  // namespace

bool AuthBySteamGroup::SDK_OnLoad(char* error, size_t maxlength, bool late) {
  SM_GET_IFACE(GAMEHELPERS, m_game_helpers);
  SM_GET_IFACE(PLAYERMANAGER, m_player_manager);
  return true;
}

void AuthBySteamGroup::SDK_OnAllLoaded() {
  sharesys->AddNatives(myself, g_natives);
}

IGamePlayer* AuthBySteamGroup::GetPlayerByUserId(int user_id) {
  int client_id = m_player_manager->GetClientOfUserId(user_id);
  if (client_id == 0) {
    return nullptr;
  }
  return m_player_manager->GetGamePlayer(client_id);
}

int AuthBySteamGroup::GetUserIdByClientId(int client_id) {
  auto* game_player = m_player_manager->GetGamePlayer(client_id);
  return game_player->GetUserId();
}

std::string AuthBySteamGroup::KickCommand(uint64_t steam_id) {
  return std::string("kickplayer ") + std::to_string(steam_id);
}

void AuthBySteamGroup::PrintToAll(const std::string& message) {
  int max_clients = m_player_manager->GetMaxClients();
  for (int client_id = 1; client_id <= max_clients; client_id++) {
    auto* game_player = m_player_manager->GetGamePlayer(client_id);
    if (!game_player->IsConnected()) {
      continue;
    }
    m_game_helpers->TextMsg(client_id, TEXTMSG_DEST_CHAT, message.c_str());
  }
}

std::future<bool> AuthBySteamGroup::CheckGroupMembership(
    int user_id, const std::string& group_id, const std::string& steam_key) {
  auto* game_player = GetPlayerByUserId(user_id);
  if (game_player == nullptr) {
    return std::async(std::launch::deferred, []() { return false; });
  }

  uint64_t steam_id64 = game_player->GetSteamId64(/*validated=*/false);
  if (steam_id64 == 0) {
    return std::async(std::launch::deferred, []() { return false; });
  }

  return std::async(
      std::launch::async, [this, user_id, steam_id64, group_id, steam_key]() {
        std::future<bool> is_member_steamworks =
            std::async(std::launch::async, CheckGroupMembershipSteamworks,
                       steam_id64, group_id, steam_key);
        std::future<bool> is_member_community =
            std::async(std::launch::async, CheckGroupMembershipCommunity,
                       steam_id64, group_id);
        if (!is_member_steamworks.get() && !is_member_community.get()) {
          return false;
        }

        auto* updated_game_player = GetPlayerByUserId(user_id);
        if (updated_game_player == nullptr) {
          return false;
        }

        uint64_t validated_steam_id64 =
            updated_game_player->GetSteamId64(/*validated=*/true);
        if (validated_steam_id64 == 0) {
          return false;
        }

        return validated_steam_id64 == steam_id64;
      });
}

void AuthBySteamGroup::AllowJoinSucceeds() {
  PrintToAll(
      "The next unauthorized player that attempts to join the server will be "
      "allowed access. Authorized players may use the 'denyjoin' command to "
      "disable this access.");
  m_allow_next_access = true;
}

void AuthBySteamGroup::AllowJoinFails(int user_id) {
  int client_id = m_player_manager->GetClientOfUserId(user_id);
  if (client_id == 0) {
    return;
  }

  m_game_helpers->TextMsg(client_id, TEXTMSG_DEST_CHAT,
                          "You are not authorized to use that command.");
}

void AuthBySteamGroup::DenyJoinSucceeds() {
  if (m_allow_next_access) {
    m_allow_next_access = false;
    PrintToAll("Unauthorized player server access disabled.");
  }
}

void AuthBySteamGroup::DenyJoinFails(int user_id) {
  int client_id = m_player_manager->GetClientOfUserId(user_id);
  if (client_id == 0) {
    return;
  }

  m_game_helpers->TextMsg(client_id, TEXTMSG_DEST_CHAT,
                          "You are not authorized to use that command.");
}

void AuthBySteamGroup::CheckAccessSucceeds(int user_id) {
  auto* game_player = GetPlayerByUserId(user_id);
  if (game_player == nullptr) {
    return;
  }

  uint64_t steam_id64 = game_player->GetSteamId64(/*validated=*/true);
  if (steam_id64 == 0) {
    return;
  }

  if (m_this_rotation_allowed_steam_ids.count(steam_id64) == 0 &&
      m_last_rotation_allowed_steam_ids.count(steam_id64) == 0) {
    int client_id = m_player_manager->GetClientOfUserId(user_id);
    if (client_id != 0) {
      m_game_helpers->TextMsg(client_id, TEXTMSG_DEST_CHAT,
                              "Use the 'allowjoin' command to allow "
                              "unauthorized players to join the server.");
    }
  }

  m_this_rotation_allowed_steam_ids.insert(steam_id64);
}

void AuthBySteamGroup::CheckAccessFails(int user_id) {
  auto* game_player = GetPlayerByUserId(user_id);
  if (game_player == nullptr) {
    return;
  }

  uint64_t steam_id64 = game_player->GetSteamId64(/*validated=*/true);
  if (steam_id64 == 0) {
    game_player->Kick("Your Steam ID could not be validated");
    return;
  }

  if (m_this_rotation_allowed_steam_ids.count(steam_id64) != 0) {
    return;
  }

  std::string kick_command = KickCommand(steam_id64);

  if (m_last_rotation_allowed_steam_ids.count(steam_id64) != 0) {
    m_this_rotation_allowed_steam_ids.insert(steam_id64);
    m_kick_command_to_user_id[kick_command] = user_id;
    return;
  }

  if (!m_allow_next_access) {
    game_player->Kick("You are not on the server allowlist");
    return;
  }

  m_allow_next_access = false;

  const char* player_name = game_player->GetName();

  if (player_name == nullptr) {
    PrintToAll("An unauthorized player was granted access.");
  } else {
    PrintToAll(std::string(player_name) + " was granted access.");
  }

  PrintToAll(
      "If this was not the player you intended to let in, authorized players "
      "may use the command '" +
      kick_command +
      "' to remove them from the server or use the command 'kicklist' to view "
      "the list of all kickable players.");

  m_this_rotation_allowed_steam_ids.insert(steam_id64);
  m_kick_command_to_user_id[kick_command] = user_id;
}

void AuthBySteamGroup::PrintKickListSucceeds(int user_id) {
  int requester_client_id = m_player_manager->GetClientOfUserId(user_id);
  if (requester_client_id == 0) {
    return;
  }

  if (m_kick_command_to_user_id.empty()) {
    m_game_helpers->TextMsg(requester_client_id, TEXTMSG_DEST_CHAT,
                            "No kickable players.");
    return;
  }

  for (const auto& entry : m_kick_command_to_user_id) {
    auto* game_player = GetPlayerByUserId(entry.second);
    if (game_player == nullptr) {
      continue;
    }

    const char* player_name = game_player->GetName();
    if (player_name == nullptr) {
      m_game_helpers->TextMsg(requester_client_id, TEXTMSG_DEST_CHAT,
                              entry.first.c_str());
    } else {
      std::string message = entry.first + ": " + player_name;
      m_game_helpers->TextMsg(requester_client_id, TEXTMSG_DEST_CHAT,
                              message.c_str());
    }
  }
}

void AuthBySteamGroup::PrintKickListFails(int user_id) {
  int client_id = m_player_manager->GetClientOfUserId(user_id);
  if (client_id == 0) {
    return;
  }

  m_game_helpers->TextMsg(client_id, TEXTMSG_DEST_CHAT,
                          "You are not authorized to use that command.");
}

void AuthBySteamGroup::KickCommandSucceeds(int to_kick_user_id) {
  auto* player_to_kick = GetPlayerByUserId(to_kick_user_id);
  if (player_to_kick == nullptr) {
    return;
  }

  uint64_t steam_id64 = player_to_kick->GetSteamId64(/*validated=*/true);
  if (steam_id64 != 0) {
    m_kick_command_to_user_id.erase(KickCommand(steam_id64));
    m_this_rotation_allowed_steam_ids.erase(steam_id64);
    m_last_rotation_allowed_steam_ids.erase(steam_id64);
  }

  const char* player_name = player_to_kick->GetName();
  if (player_name == nullptr) {
    PrintToAll(std::string(player_name) + " was kicked.");
  } else {
    PrintToAll("Player kicked.");
  }

  player_to_kick->Kick("You have been kicked from the server");
}

void AuthBySteamGroup::KickCommandFails(int requester_user_id) {
  int client_id = m_player_manager->GetClientOfUserId(requester_user_id);
  if (client_id == 0) {
    return;
  }

  m_game_helpers->TextMsg(client_id, TEXTMSG_DEST_CHAT,
                          "You are not authorized to use that command.");
}

void AuthBySteamGroup::AllowJoin(int client_id, std::string group_id,
                                 std::string steam_key) {
  int user_id = GetUserIdByClientId(client_id);
  auto async_op = std::async(
      std::launch::async,
      [this, user_id, group_id, steam_key]() -> std::function<void()> {
        auto is_group_member =
            CheckGroupMembership(user_id, group_id, steam_key);
        if (!is_group_member.get()) {
          return std::bind(&AuthBySteamGroup::AllowJoinFails, this, user_id);
        }

        return std::bind(&AuthBySteamGroup::AllowJoinSucceeds, this);
      });

  std::lock_guard<std::mutex> lock(m_plugin_lock);
  m_async_operations.push_back(std::move(async_op));
}

void AuthBySteamGroup::DenyJoin(int client_id, std::string group_id,
                                std::string steam_key) {
  int user_id = GetUserIdByClientId(client_id);
  auto async_op = std::async(
      std::launch::async,
      [this, user_id, group_id, steam_key]() -> std::function<void()> {
        auto is_group_member =
            CheckGroupMembership(user_id, group_id, steam_key);
        if (!is_group_member.get()) {
          return std::bind(&AuthBySteamGroup::DenyJoinFails, this, user_id);
        }

        return std::bind(&AuthBySteamGroup::DenyJoinSucceeds, this);
      });

  std::lock_guard<std::mutex> lock(m_plugin_lock);
  m_async_operations.push_back(std::move(async_op));
}

void AuthBySteamGroup::CheckAccess(int client_id, std::string group_id,
                                   std::string steam_key) {
  int user_id = GetUserIdByClientId(client_id);
  auto async_op = std::async(
      std::launch::async,
      [this, user_id, group_id, steam_key]() -> std::function<void()> {
        auto is_group_member =
            CheckGroupMembership(user_id, group_id, steam_key);
        if (!is_group_member.get()) {
          return std::bind(&AuthBySteamGroup::CheckAccessFails, this, user_id);
        }

        return std::bind(&AuthBySteamGroup::CheckAccessSucceeds, this, user_id);
      });

  std::lock_guard<std::mutex> lock(m_plugin_lock);
  m_async_operations.push_back(std::move(async_op));
}

void AuthBySteamGroup::PrintKickList(int client_id, std::string group_id,
                                     std::string steam_key) {
  int user_id = GetUserIdByClientId(client_id);
  auto async_op = std::async(
      std::launch::async,
      [this, user_id, group_id, steam_key]() -> std::function<void()> {
        auto is_group_member =
            CheckGroupMembership(user_id, group_id, steam_key);
        if (!is_group_member.get()) {
          return std::bind(&AuthBySteamGroup::PrintKickListFails, this,
                           user_id);
        }

        return std::bind(&AuthBySteamGroup::PrintKickListSucceeds, this,
                         user_id);
      });

  std::lock_guard<std::mutex> lock(m_plugin_lock);
  m_async_operations.push_back(std::move(async_op));
}

bool AuthBySteamGroup::TryProcessKickCommand(int client_id,
                                             std::string group_id,
                                             std::string steam_key,
                                             const char* text) {
  std::lock_guard<std::mutex> lock(m_plugin_lock);
  auto iter = m_kick_command_to_user_id.find(text);
  if (iter == m_kick_command_to_user_id.end()) {
    return false;
  }

  int requester_user_id = GetUserIdByClientId(client_id);
  int to_kick_user_id = iter->second;

  auto async_op =
      std::async(std::launch::async,
                 [this, requester_user_id, to_kick_user_id, group_id,
                  steam_key]() -> std::function<void()> {
                   auto is_group_member = CheckGroupMembership(
                       requester_user_id, group_id, steam_key);
                   if (!is_group_member.get()) {
                     return std::bind(&AuthBySteamGroup::KickCommandFails, this,
                                      requester_user_id);
                   }

                   return std::bind(&AuthBySteamGroup::KickCommandSucceeds,
                                    this, to_kick_user_id);
                 });

  m_async_operations.push_back(std::move(async_op));

  return true;
}

void AuthBySteamGroup::RotateAccessGrants() {
  std::lock_guard<std::mutex> lock(m_plugin_lock);
  std::swap(m_this_rotation_allowed_steam_ids,
            m_last_rotation_allowed_steam_ids);
  m_this_rotation_allowed_steam_ids.clear();
  m_kick_command_to_user_id.clear();
  m_allow_next_access = false;
}

void AuthBySteamGroup::OnFrame() {
  std::lock_guard<std::mutex> lock(m_plugin_lock);
  std::remove_if(m_async_operations.begin(), m_async_operations.end(),
                 [](std::future<std::function<void()>>& item) {
                   if (!item.valid()) {
                     return false;
                   }

                   item.get()();
                   return true;
                 });
}

SMEXT_LINK(&g_auth_by_steam_group);