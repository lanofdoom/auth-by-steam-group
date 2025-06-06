# LAN of DOOM Authenticate By Steam Group Plugin
SourceMod extension and plugin for SRCDS servers that together only permits
members of a specified steam groups to join the server.

Note, servers with ``sv_lan 1`` are not compatible with Steam Group
authentication.

# Building
Check out the repository, install the dependencies listed in ``build.sh``, and
then run the ``./build.sh`` script.

# Installation
1) If running Linux, install the ``ca-certificates`` and ``curl`` packages or
   their equivalent on your distribution to your server.
2) If running *64-bit* Linux and the dedicated server is a 32-bit binary,
   install the ``libstdc++6:i386`` package or its equivalent on your
   distribution to your server.
3) Extract ``auth_by_steam_group_source.tar.gz`` or
   ``auth_by_steam_group_source.zip`` in your server's ``cstrike`` directory or
   equivalent.

# Console Variables
``sm_auth_by_steam_allowjoin_enabled`` If enabled, members of the authorized
Steam group can allow the next unauthorized user to join the server by saying
`allowjoin` in chat. Default value is enabled.

``sm_auth_by_steam_group_group_id`` Sets the ID of the Steam group from which
players are allowed to join. If empty, all players are allowed to join. Default
value is empty.

``sm_auth_by_steam_group_steam_key`` Sets the
[Steam API key](https://steamcommunity.com/dev/apikey) for used for the group
membership lookups. If empty, all players are allowed to join. Default value is
empty.
