## Releases

* [Latest](https://lanofdoom.github.io/auth-by-steam-group/releases/latest/auth_by_steam_group.tar.gz) ([Source](https://lanofdoom.github.io/auth-by-steam-group/releases/latest/auth_by_steam_group_source.tar.gz)) (1.0.0)
* [Nightly](https://lanofdoom.github.io/auth-by-steam-group/releases/nightly/auth_by_steam_group.tar.gz) ([Source](https://lanofdoom.github.io/auth-by-steam-group/releases/nightly/auth_by_steam_group_source.tar.gz)) 

## Installation
1) Install the ``libcurl:i386`` package or its equivalent on your distribution to your server.
2) Extract ``auth_by_steam_group_source.tar.gz`` in your server's ``css/cstrike`` directory.

Note, servers with ``sv_lan 1`` are not compatible with Steam Group authentication and will allow all users to join regardless of how the plugin or its cvars are configured.

## Console Variables
``sm_auth_by_steam_group_group_id`` Sets the ID of the group from which players are allowed to join. If empty, all players are allowed to join. Default value is empty.

``sm_auth_by_steam_group_steam_key`` Sets the [Steam API key](https://steamcommunity.com/dev/apikey) for used for the group membership lookups. If empty, all players are allowed to join. Default value is empty.

## Version History

### 1.0.0 ([Download](https://lanofdoom.github.io/auth-by-steam-group/releases/v1.0.0/auth_by_steam_group.tar.gz)) ([Source](https://lanofdoom.github.io/auth-by-steam-group/releases/v1.0.0/auth_by_steam_group_source.tar.gz)) 
* Initial Release
