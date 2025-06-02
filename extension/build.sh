#!/bin/bash -ue

# Build SourceMod @ HEAD.
# TODO: Figure out how to pin this to a stable version.
#
# Required Packages:
# clang lib32stdc++-7-dev lib32z1-dev libc6-dev-i386 linux-libc-dev g++-multilib
# python (python2)
#
# Copied from https://wiki.alliedmods.net/Building_SourceMod

mkdir -p ../alliedmodders/extension/build
cp -r ./* ../alliedmodders/extension/
cd ../alliedmodders/extension/build
git clone https://github.com/alliedmodders/hl2sdk-manifests.git ../hl2sdk-manifests
CC=clang CXX=clang++ python3 ../configure.py --hl2sdk-manifest-path ./hl2sdk-manifests
ambuild

mkdir -p ../../../build/addons/sourcemod/extensions
cp -r package/addons/sourcemod/extensions/* ../../../build/addons/sourcemod/extensions/

echo built auth_by_source_id.ext.so

cd ../../extension