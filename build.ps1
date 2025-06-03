mkdir -p alliedmodders
cd alliedmodders
git clone --recursive https://github.com/alliedmodders/sourcemod
cd sourcemod
git checkout 5c407d494889dd1b6b2187bbc31ac82 && git clean -fd
cd ..
bash sourcemod/tools/checkout-deps.sh
cd mmsource-1.12
git clean -fd && git checkout 02ee4a312525d17b2f6d2f21a953d06
cd ..

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