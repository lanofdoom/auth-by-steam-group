# LAN of DOOM Authenticate By Steam Group Plugin
SourceMod extension and plugin for SRCDS servers that together only permits members of a specified steam groups to join the server.

Note, servers with ``sv_lan 1`` are not compatible with Steam Group authentication.

# Building
Install [Bazelisk](https://github.com/bazelbuild/bazelisk), then build for your server's platform (`windows_x86_32`, `linux_x86_32`, or the `_x86_64` variants):

```console
$ bazel build --platforms=@rules_sourcemod//platforms:windows_x86_32 //:auth_by_steam_group_tar
```

Windows targets build natively on Windows. To cross-compile ``windows_x86_32`` from Linux, also pass ``--extra_toolchains=@llvm_toolchain_windows_x86_32//:cc-toolchain-x86_32-windows``.

The output is ``bazel-bin/auth_by_steam_group.tar.gz``.

# Installation
1) If running Linux, install the ``ca-certificates`` package or its equivalent on your distribution to your server. Certificates are read from ``/etc/ssl/certs``.
2) If running *64-bit* Linux and the dedicated server is a 32-bit binary, install the ``libstdc++6:i386`` package or its equivalent on your distribution to your server.
3) Extract ``auth_by_steam_group.tar.gz`` in your server's ``cstrike`` directory or equivalent.

``libcurl`` is linked statically on both Linux and Windows, so no external ``curl`` or ``libcurl`` package is needed.

# Console Variables
``sm_auth_by_steam_allowjoin_enabled`` If enabled, members of the authorized Steam group can allow the next unauthorized user to join the server by saying `allowjoin` in chat. Default value is enabled.

``sm_auth_by_steam_group_group_id`` Sets the ID of the Steam group from which players are allowed to join. If empty, all players are allowed to join. Default value is empty.

``sm_auth_by_steam_group_steam_key`` Sets the [Steam API key](https://steamcommunity.com/dev/apikey) for used for the group membership lookups. If empty, all players are allowed to join. Default value is empty.
