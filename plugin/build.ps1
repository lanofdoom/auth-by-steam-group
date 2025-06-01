Function New-TemporaryFolder {
    $File = New-TemporaryFile
    Remove-Item $File -Force
    New-Item -Itemtype Directory -Path "$($ENV:Temp)\$($File.Name)" 
}

$tempdir = New-TemporaryFolder
$tools = 'https://sm.alliedmods.net/smdrop/1.12/sourcemod-1.12.0-git7206-windows.zip'
$tools_download_path = $tempdir.FullName + '\tools.zip'

& curl $tools -o $tools_download_path
Expand-Archive -LiteralPath $tools_download_path -DestinationPath $tempdir.FullName

$spcomp = $tempdir.FullName + '\addons\sourcemod\scripting\spcomp'
& $spcomp auth_by_steam_group.sp

mkdir -p ../build/addons/sourcemod/plugins
move auth_by_steam_group.smx ../build/addons/sourcemod/plugins/auth_by_steam_group.smx

echo built auth_by_steam_group.smx