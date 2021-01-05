#!/bin/bash -ue

# Build SourceMod @ HEAD.
# TODO: Figure out how to pin this to a stable version.
#
# Required Packages:
# clang lib32stdc++-7-dev lib32z1-dev libc6-dev-i386 linux-libc-dev g++-multilib
# python (python2)
#
# Copied from https://wiki.alliedmods.net/Building_SourceMod
mkdir -p alliedmodders
cd alliedmodders
git clone --recursive https://github.com/alliedmodders/sourcemod
bash sourcemod/tools/checkout-deps.sh
cd sourcemod
mkdir build
cd build
CC=clang CXX=clang++ python ../configure.py
ambuild
cd ../..