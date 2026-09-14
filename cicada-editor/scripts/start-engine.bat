@echo off
rem M1c dev stack: start cicada-engine (dynamic port + testlib + scratch copy).
set EXE=C:\dsh\Schematic-Cicada\cicada-editor\build-msvc-kicad\Release\cicada-engine.exe
rem AI truth file = ACTIVE session workspace (host 会话 cwd = C:\dsh\cicada；launcher 一期由活动会话解析)。
set WORK=C:\dsh\Schematic-Cicada\cicada-editor\build-msvc-kicad\m1c-work
set SCH=C:\dsh\cicada\.cicada\schematic.cicada_sch
if not exist "%WORK%" mkdir "%WORK%" >nul 2>&1
if not exist "%SCH%" if exist "C:\dsh\Schematic-Cicada\cicada-harness\packages\cicada\cicada-format\tests\fixtures\minimal.cicada_sch" copy /y "C:\dsh\Schematic-Cicada\cicada-harness\packages\cicada\cicada-format\tests\fixtures\minimal.cicada_sch" "%SCH%" >nul 2>&1
start /b "" "%EXE%" --port 0 --file "%SCH%" --lib-dir "C:\dsh\Schematic-Cicada\assets\cicada-libs" --log "%WORK%\engine.txt"
