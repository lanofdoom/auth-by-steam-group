# LAN of DOOM Authenticate By Steam Group Plugin
SourceMod extension and plugin for **Linux** SRCDS servers that together only permits members of a specified steam groups to join the server.

Note, servers with ``sv_lan 1`` are not compatible with Steam Group authentication and will allow all users to join regardless of how the plugin or its cvars are configured.

# Building
Check out the repository, install the dependencies listed in ``build.sh``, and then run the ``./build.sh`` script.

# Installation
1) Install the ``libcurl:i386`` package or its equivalent on your distribution to your server.
2) Extract ``auth_by_steam_group_source.tar.gz`` in your server's ``css/cstrike`` directory.

Note, servers with ``sv_lan 1`` are not compatible with Steam Group authentication and will allow all users to join regardless of how the plugin or its cvars are configured.

# Console Variables
``sm_auth_by_steam_group_group_id`` A comma-separate list of the IDs of the Steam groups allowed on the server. If empty, all players are allowed to join. Default value is empty.

``sm_auth_by_steam_group_steam_key`` Sets the [Steam API key](https://steamcommunity.com/dev/apikey) for used for the group membership lookups. If empty, all players are allowed to join. Default value is empty.