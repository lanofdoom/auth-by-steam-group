mkdir -p ..\alliedmodders\extension\build
cp -r .\* ..\alliedmodders\extension\
cd ..\alliedmodders\extension\build
python ..\configure.py
ambuild

mkdir -p ..\..\..\build\addons\sourcemod\extensions
cp package\addons\sourcemod\extensions\* ..\..\..\build\addons\sourcemod\extensions\

echo built auth_by_source_id.ext.dll

cd ..\..\extension