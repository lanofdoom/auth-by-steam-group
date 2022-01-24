Function New-TemporaryFolder {
    $File = New-TemporaryFile
    Remove-Item $File -Force
    New-Item -Itemtype Directory -Path "$($ENV:Temp)\$($File.Name)" 
}

$tools = 'https://sm.alliedmods.net/smdrop/1.10/sourcemod-1.10.0-git6528-windows.zip'

$tempdir = New-TemporaryFolder

curl $tools -o $tmp_dir\tools.zip
7z x $tmp_dir/tools.tar.gz -o $tmp_dir
$tmp_dir/addons/sourcemod/scripting/spcomp auth_by_steam_group.sp

mkdir -p ../build/addons/sourcemod/plugins
move auth_by_steam_group.smx ../build/addons/sourcemod/plugins/auth_by_steam_group.smx

echo built auth_by_steam_group.smx