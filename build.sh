#!/bin/bash -ue

# Build SourceMod @ HEAD.
# TODO: Figure out how to pin this to a stable version.
#
# Required Packages:
# clang lib32stdc++-7-dev lib32z1-dev libc6-dev-i386 linux-libc-dev g++-multilib
# python (python2) libcurl4-openssl-dev:i386
#
# Copied from https://wiki.alliedmods.net/Building_SourceMod
mkdir -p alliedmodders
cd alliedmodders
git clone --recursive https://github.com/alliedmodders/sourcemod
bash sourcemod/tools/checkout-deps.sh

cd ../extension
./build.sh
cd ../plugin
./build.sh
cd ..

tar -czf auth_by_steam_group_source.tar.gz extension plugin build.ps1 build.sh LICENSE README.md

cd build
tar -czf auth_by_steam_group.tar.gz addons
mkdir -p dist
mv auth_by_steam_group.tar.gz dist/auth_by_steam_group.tar.gz
mv ../auth_by_steam_group_source.tar.gz dist/auth_by_steam_group_source.tar.gz
cd ..