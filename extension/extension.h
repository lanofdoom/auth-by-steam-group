#ifndef _AUTH_BY_STEAM_GROUP_EXTENSION_EXTENSION_
#define _AUTH_BY_STEAM_GROUP_EXTENSION_EXTENSION_

#include <IGameHelpers.h>
#include <IPlayerHelpers.h>

#include <functional>
#include <future>
#include <map>
#include <mutex>
#include <set>
#include <vector>

#include "smsdk_ext.h"

class AuthBySteamGroup : public SDKExtension {
 public:
  virtual bool SDK_OnLoad(char *error, size_t maxlength, bool late);
  virtual void SDK_OnAllLoaded();

  void AllowAccess(int client_id, std::string group_id, std::string steam_key);
  void CheckAccess(int client_id, std::string group_id, std::string steam_key);
  void PrintKickList(int client_id, std::string group_id,
                     std::string steam_key);

  bool TryProcessKickCommand(int client_id, std::string group_id,
                             std::string steam_key, const char* text);

  void RotateAccessGrants();
  void OnFrame();

 private:
  std::future<bool> CheckGroupMembership(int client_id,
                                         const std::string& group_id,
                                         const std::string& steam_key);
  IGamePlayer* GetPlayerByUserId(int user_id);
  int GetUserIdByClientId(int client_id);
  void PrintToAll(const std::string& message);
  std::string KickCommand(uint64_t steam_id);

  // Callbacks for allow access
  void AllowAccessSucceeds();
  void AllowAccessFails(int user_id);

  // Callbacks for check access
  void CheckAccessSucceeds(int user_id);
  void CheckAccessFails(int user_id);

  // Callbacks for print kick list
  void PrintKickListSucceeds(int user_id);
  void PrintKickListFails(int user_id);

  // Callbacks for print kick list
  void KickCommandSucceeds(int to_kick_user_id);
  void KickCommandFails(int requester_user_id);

  IGameHelpers* m_game_helpers;
  IPlayerManager* m_player_manager;

  std::mutex m_plugin_lock;
  bool m_allow_next_access = false;
  std::map<std::string, int> m_kick_command_to_user_id;
  std::set<uint64_t> m_this_rotation_allowed_steam_ids;
  std::set<uint64_t> m_last_rotation_allowed_steam_ids;
  std::vector<std::future<std::function<void()>>> m_async_operations;
};

#endif  // _AUTH_BY_STEAM_GROUP_EXTENSION_EXTENSION_
