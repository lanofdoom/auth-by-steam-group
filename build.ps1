mkdir -p alliedmodders
cd alliedmodders
git clone --recursive https://github.com/alliedmodders/sourcemod
.\sourcemod\tools\checkout-deps.ps1

cd ..\extension
.\build.ps1
cd ..\plugin
.\build.ps1
cd ..

cd build
7z a -tzip auth_by_steam_group.zip addons
mkdir -p dist
move auth_by_steam_group.zip dist\auth_by_steam_group.zip
cd ..