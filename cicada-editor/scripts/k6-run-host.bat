@echo off
rem replicate the wx-app's exact child spawn (same env/cwd/argv) and capture output
set CICADA_DSH_NODE=C:\Program Files\nodejs\node.exe
set CICADA_DSH_ENTRY=C:\dsh\Schematic-Cicada\cicada-harness\apps\cli\src\bin.ts
set CICADA_DSH_PRELOAD=tsx/esm
set CICADA_DSH_HOME=C:\dsh\Schematic-Cicada\cicada-dev-home
set DSH_HOME=C:\dsh\Schematic-Cicada\cicada-dev-home
set CICADA_HOME=C:\dsh\Schematic-Cicada\cicada-dev-home
cd /d C:\dsh\Schematic-Cicada\cicada-harness
"%CICADA_DSH_NODE%" --import tsx/esm "%CICADA_DSH_ENTRY%" --profile cicada --no-open --port 3123 --host 127.0.0.1
