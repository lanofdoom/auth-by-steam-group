## Releases

* [Latest](https://lanofdoom.github.io/auth-by-steam-group/releases/latest/auth_by_steam_group.tar.gz) ([Source](https://lanofdoom.github.io/auth-by-steam-group/releases/latest/auth_by_steam_group_source.tar.gz)) (1.2.1)
* [Nightly](https://lanofdoom.github.io/auth-by-steam-group/releases/nightly/auth_by_steam_group.tar.gz) ([Source](https://lanofdoom.github.io/auth-by-steam-group/releases/nightly/auth_by_steam_group_source.tar.gz)) 

## Installation
1.  Install the ``libcurl:i386`` package or its equivalent on your distribution to your server.
2.  Extract ``auth_by_steam_group_source.tar.gz`` in your server's ``css/cstrike`` directory.

Note, servers with ``sv_lan 1`` are not compatible with Steam Group authentication and will allow all users to join regardless of how the plugin or its cvars are configured.

## Console Variables
``sm_auth_by_steam_allowaccess_enabled`` If enabled, members of the authorized Steam group can allow access to the next unauthorized user that attempts to join the server by saying ``allowaccess`` in chat. Default value is disabled.

``sm_auth_by_steam_group_group_id`` A comma-separated list of the ID of the Steam groups whose members are allowed to join the server. If empty, all players are allowed to join. Default value is empty.

``sm_auth_by_steam_group_steam_key`` Sets the [Steam API key](https://steamcommunity.com/dev/apikey) for used for the group membership lookups. If empty, all players are allowed to join. Default value is empty.

## Version History

### 1.2.1 ([Download](https://lanofdoom.github.io/auth-by-steam-group/releases/v1.2.1/auth_by_steam_group.tar.gz)) ([Source](https://lanofdoom.github.io/auth-by-steam-group/releases/v1.2.1/auth_by_steam_group_source.tar.gz))
* Allow unauthorized players to be kicked from the server by any authorized players.
* Allow unauthorized players to remain connected across map changes.

### 1.2.0 ([Download](https://lanofdoom.github.io/auth-by-steam-group/releases/v1.2.0/auth_by_steam_group.tar.gz)) ([Source](https://lanofdoom.github.io/auth-by-steam-group/releases/v1.2.0/auth_by_steam_group_source.tar.gz))
* Adds `sm_auth_by_steam_allowaccess_enabled` cvar

### 1.1.0 ([Download](https://lanofdoom.github.io/auth-by-steam-group/releases/v1.1.0/auth_by_steam_group.tar.gz)) ([Source](https://lanofdoom.github.io/auth-by-steam-group/releases/v1.1.0/auth_by_steam_group_source.tar.gz))
* Enables `sm_auth_by_steam_group_steam_key` to be a comma-separated list of Steam group IDs

### 1.0.1 ([Download](https://lanofdoom.github.io/auth-by-steam-group/releases/v1.0.1/auth_by_steam_group.tar.gz)) ([Source](https://lanofdoom.github.io/auth-by-steam-group/releases/v1.0.1/auth_by_steam_group_source.tar.gz))
* Punctuation and wording fixes

### 1.0.0 ([Download](https://lanofdoom.github.io/auth-by-steam-group/releases/v1.0.0/auth_by_steam_group.tar.gz)) ([Source](https://lanofdoom.github.io/auth-by-steam-group/releases/v1.0.0/auth_by_steam_group_source.tar.gz))
* Initial Release
