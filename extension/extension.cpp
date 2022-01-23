#include "extension.h"

#include <i386-linux-gnu/curl/curl.h>

#include <algorithm>
#include <optional>

namespace {

AuthBySteamGroup g_auth_by_steam_group;

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
// given the simplicity of the response, but this is worth fixing at some point.
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

std::string BuildSteamworksQueryUrl(uint64_t steam_id64,
                                    const std::string& steam_key) {
  std::string url =
      "https://api.steampowered.com/ISteamUser/GetUserGroupList/v1/?steamid=";
  url += std::to_string(steam_id64);
  url += "&key=";
  url += steam_key;
  return url;
}

std::string BuildCommunityQueryUrl(const std::string& steam_group_id) {
  std::string url = "https://steamcommunity.com/gid/[G:1:";
  url += steam_group_id;
  url += "]/memberslistxml/?xml=1";
  return url;
}

size_t WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
  std::string* data = reinterpret_cast<std::string*>(userdata);
  for (size_t i = 0; i < size * nmemb; i++) {
    data->push_back(ptr[i]);
  }
  return size * nmemb;
}

std::optional<std::string> DoHttpRequest(const std::string& url) {
  CURL* curl = curl_easy_init();
  if (curl == nullptr) {
    return std::nullopt;
  }

  CURLcode code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  if (code != CURLE_OK) {
    curl_easy_cleanup(curl);
    return std::nullopt;
  }

  std::string data;
  code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
  if (code != CURLE_OK) {
    curl_easy_cleanup(curl);
    return std::nullopt;
  }

  code = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  if (code != CURLE_OK) {
    curl_easy_cleanup(curl);
    return std::nullopt;
  }

  code = curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
  if (code != CURLE_OK) {
    curl_easy_cleanup(curl);
    return std::nullopt;
  }

  code = curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
  if (code != CURLE_OK) {
    curl_easy_cleanup(curl);
    return std::nullopt;
  }

  code = curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  if (code != CURLE_OK) {
    return std::nullopt;
  }

  return data;
}

bool CheckGroupMembershipSteamworks(uint64_t steam_id64,
                                    const std::string& steam_group_id,
                                    const std::string& steam_api_key) {
  std::string request = BuildSteamworksQueryUrl(steam_id64, steam_api_key);
  std::optional<std::string> response = DoHttpRequest(request);
  if (!response.has_value()) {
    return false;
  }
  std::set<std::string> client_group_ids = ParseGroupIds(response.get());
  return client_group_ids.count(steam_group_id) != 0;
}

bool CheckGroupMembershipCommunity(uint64_t steam_id64,
                                   const std::string& steam_group_id) {
  std::string request = BuildCommunityQueryUrl(steam_group_id);
  std::optional<std::string> response = DoHttpRequest(request);
  if (!response.has_value()) {
    return false;
  }
  return response.get().find(std::to_string(steam_id64)) != string::npos;
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