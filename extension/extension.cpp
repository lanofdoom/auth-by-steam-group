#include "extension.h"

#include <i386-linux-gnu/curl/curl.h>

#include <memory>
#include <set>
#include <string>
#include <vector>

namespace {

AuthBySteamGroup g_auth_by_steam_group;

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

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
  std::string* data = reinterpret_cast<std::string*>(userdata);
  for (size_t i = 0; i < size * nmemb; i++) {
    data->push_back(ptr[i]);
  }
  return size * nmemb;
}

cell_t CheckUser(IPluginContext* context, const cell_t* params) {
  char* network_id;
  context->LocalToString(params[2], &network_id);

  char* group_id;
  context->LocalToString(params[3], &group_id);

  char* steam_key;
  context->LocalToString(params[4], &steam_key);

  CURL* curl = curl_easy_init();
  if (curl == nullptr) {
    return true;
  }

  CURLcode code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  if (code != CURLE_OK) {
    curl_easy_cleanup(curl);
    return true;
  }

  std::string data;
  code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
  if (code != CURLE_OK) {
    curl_easy_cleanup(curl);
    return true;
  }

  std::string query_url = BuildQueryUrl(network_id, steam_key);
  code = curl_easy_setopt(curl, CURLOPT_URL, query_url.c_str());
  if (code != CURLE_OK) {
    curl_easy_cleanup(curl);
    return true;
  }

  code = curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
  if (code != CURLE_OK) {
    curl_easy_cleanup(curl);
    return true;
  }

  code = curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
  if (code != CURLE_OK) {
    curl_easy_cleanup(curl);
    return true;
  }

  code = curl_easy_perform(curl);
  if (code != CURLE_OK) {
    curl_easy_cleanup(curl);
    return true;
  }

  std::set<std::string> group_ids = ParseGroupIds(data);
  if (group_ids.count(group_id) != 0) {
    curl_easy_cleanup(curl);
    return true;
  }

  curl_easy_cleanup(curl);

  return false;
}

const sp_nativeinfo_t g_natives[] = {
    {"AuthBySteamGroup_CheckUser", CheckUser},
    {nullptr, nullptr},
};

}  // namespace

void AuthBySteamGroup::SDK_OnAllLoaded() {
  sharesys->AddNatives(myself, g_natives);
}

SMEXT_LINK(&g_auth_by_steam_group);