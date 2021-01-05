#include "extension.h"

#include <IWebternet.h>
#include <amtl/am-thread.h>

#include <memory>
#include <set>
#include <string>
#include <vector>

namespace {

IWebternet* g_webternet = nullptr;

AuthBySteamGroup g_auth_by_steam_group;
SMEXT_LINK(&g_auth_by_steam_group);

class StringTransferHandler : public ITransferHandler {
 public:
  DownloadWriteStatus OnDownloadWrite(IWebTransfer* session, void* userdata,
                                      void* ptr, size_t size, size_t nmemb) {
    char* bytes = reinterpret_cast<char*>(ptr);
    for (size_t i = 0; i < size * nmemb; i++) {
      m_data.push_back(bytes[i]);
    }
    return DownloadWrite_Okay;
  }

  const std::string& data() { return m_data; }

 private:
  std::string m_data;
};

// This is a bit of a hack. Instead of properly parsing out the JSON, just
// break it up into tokens and iterate through them. This should be fine
// given the simplicity of the response, but this is worth fixing at some point.
std::set<std::string> ParseGroupIds(const std::string& response) {
  const static std::string delims = "{}[]:\",";

  std::vector<std::string> tokens;
  bool new_token = true;
  for (char c : response) {
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

std::string BuildQueryUrl(const std::string& network_id,
                          const std::string& steam_key) {
  std::string url =
      "https://api.steampowered.com/ISteamUser/GetUserGroupList/v1/?steamid=";
  url += network_id;
  url += "&key=";
  url += steam_key;
  return url;
}

void CheckUserThread(IPluginContext* context,
                     std::unique_ptr<IWebTransfer> web_transfer,
                     std::string network_id, std::string group_id,
                     std::string steam_key, funcid_t not_a_member_callback) {
  web_transfer->SetFailOnHTTPError(true);

  std::string query_url = BuildQueryUrl(network_id, steam_key);

  StringTransferHandler transfer_handler;
  if (!web_transfer->Download(query_url.c_str(), &transfer_handler, nullptr)) {
    return;
  }

  std::set<std::string> group_ids = ParseGroupIds(transfer_handler.data());
  if (group_ids.count(group_id) != 0) {
    return;
  }

  IPluginFunction* func = context->GetFunctionById(not_a_member_callback);
  if (func == nullptr) {
    return;
  }

  func->PushString(network_id.c_str());
  func->Execute(nullptr);
}

cell_t CheckUser(IPluginContext* context, const cell_t* params) {
  char* network_id;
  context->LocalToString(params[1], &network_id);

  char* group_id;
  context->LocalToString(params[2], &group_id);

  char* steam_key;
  context->LocalToString(params[3], &steam_key);

  std::unique_ptr<std::thread> thread =
      ke::NewThread("CheckGroupMembershipThread", CheckUserThread,
                    std::unique_ptr<IWebTransfer>(g_webternet->CreateSession()),
                    network_id, group_id, steam_key, params[4]);
  thread->detach();

  return 0;
}

const sp_nativeinfo_t g_natives[] = {
    {"AuthBySteamGroup_CheckUser", CheckUser},
    {nullptr, nullptr},
};

}  // namespace

bool AuthBySteamGroup::SDK_OnLoad(char* error, size_t maxlen, bool late) {
  sharesys->AddDependency(myself, "webternet.ext", true, true);
  return true;
}

void AuthBySteamGroup::SDK_OnAllLoaded() {
  SM_GET_LATE_IFACE(WEBTERNET, g_webternet);
  sharesys->AddNatives(myself, g_natives);
}

bool AuthBySteamGroup::QueryRunning(char* error, size_t maxlength) {
  SM_CHECK_IFACE(WEBTERNET, g_webternet);
  return true;
}