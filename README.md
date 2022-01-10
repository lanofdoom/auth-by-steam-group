# LAN of DOOM Authenticate By Steam Group Plugin
SourceMod extension and plugin for **Linux** SRCDS servers that together only permits members of a specified steam groups to join the server.

Note, servers with ``sv_lan 1`` are not compatible with Steam Group authentication and will allow all users to join regardless of how the plugin or its cvars are configured.

# Building
Check out the repository, install the dependencies listed in ``build.sh``, and then run the ``./build.sh`` script.

# Installation
1) Install the ``libcurl:i386`` package or its equivalent on your distribution to your server.
2) Extract ``auth_by_steam_group_source.tar.gz`` in your server's ``cstrike`` directory or equivalent.

# Console Variables
``sm_auth_by_steam_allowjoin_enabled`` If enabled, members of the authorized Steam group can allow the next unauthorized user to join the server by saying `allowjoin` in chat. Default value is enabled.

``sm_auth_by_steam_group_group_id`` Sets the ID of the Steam group from which players are allowed to join. If empty, all players are allowed to join. Default value is empty.

``sm_auth_by_steam_group_steam_key`` Sets the [Steam API key](https://steamcommunity.com/dev/apikey) for used for the group membership lookups. If empty, all players are allowed to join. Default value is empty.