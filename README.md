# LAN of DOOM Authenticate By Steam Group Plugin
SourceMod extension and plugin that together only permits members of a specified steam group to join the server.

Note, servers with ``sv_lan 1`` are not compatible with Steam Group authentication and will allow all users to join regardless of how the plugin or its cvars are configured.

Additionally, it's important to be aware that internally the extension check for Steam group membership using **unencypted** HTTP. This means that the the Steam key sent along with the request will be exposed in plaintext and as such it's best to use the API key from a Steam account with no value monetary or otherwise. This property is obviously undesireable and may be resolved in future versions of the extension.

# Building
Check out the repository and run the ``./build.sh`` script.

# Installation
Copy ``auth_by_steam_group.smx`` to your server's ``css/cstrike/addons/sourcemod/plugins`` directory.

# Console Variables
``sm_auth_by_steam_group_group_id`` Sets the ID of the group from which players are allowed to join. If empty, all players are allowed to join. Default value is empty.

``sm_auth_by_steam_group_steam_key`` Sets the [Steam API key](https://steamcommunity.com/dev/apikey) for used for the group membership lookups. If empty, all players are allowed to join. Default value is empty.