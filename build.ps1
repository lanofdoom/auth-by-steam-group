mkdir -p alliedmodders
cd alliedmodders
git clone --recursive https://github.com/alliedmodders/sourcemod
cd sourcemod
git checkout sourcemod-1.7.2
cd ..
.\sourcemod\tools\checkout-deps.ps1

cd ..\extension
.\build.ps1
cd ..\plugin
.\build.ps1
cd ..

cd build
Compress-Archive -Path addons -DestinationPath auth_by_steam_group.zip
mkdir -p dist
move auth_by_steam_group.zip dist\auth_by_steam_group.zip
cd ..