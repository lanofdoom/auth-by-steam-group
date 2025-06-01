#!/bin/bash -ue

tools=https://sm.alliedmods.net/smdrop/1.12/sourcemod-1.12.0-git7206-linux.tar.gz

tmp_dir=$(mktemp -d)
echo using temporary folder: $tmp_dir >&2
clean () {
    rm -rf $tmp_dir
    echo deleted temporary folder >&2
}
trap clean EXIT

curl $tools -o $tmp_dir/tools.tar.gz
tar -xf $tmp_dir/tools.tar.gz -C $tmp_dir
$tmp_dir/addons/sourcemod/scripting/spcomp auth_by_steam_group.sp

mkdir -p ../build/addons/sourcemod/plugins
mv auth_by_steam_group.smx ../build/addons/sourcemod/plugins/auth_by_steam_group.smx

echo built auth_by_steam_group.smx