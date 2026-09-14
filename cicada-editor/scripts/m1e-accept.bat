@echo off
rem M1 acceptance headless run: engine + shape.json workspace + dsh cicada-headless session + export + kicad-cli
setlocal EnableDelayedExpansion
set ROOT=C:\dsh\Schematic-Cicada
set HOME=%ROOT%\cicada-harness\.cicada-custom-provider-live
set WORK=%HOME%\m1e-accept
set EXE=%ROOT%\cicada-editor\build-msvc-kicad\Release\cicada-engine.exe
set KICAD="C:\Program Files\KiCad\10.0\bin\kicad-cli.exe"
set LOGDIR=%ROOT%\cicada-editor\build-msvc-kicad
set USERLIB=%HOME%\cicada-user-lib

if exist "%WORK%" rd /s /q "%WORK%"
mkdir "%WORK%\.cicada" "%WORK%\datasheet\AMS1117" "%USERLIB%" >nul 2>&1

taskkill /F /IM cicada-engine.exe >nul 2>&1
start /b "" "%EXE%" --port 0 --file "%WORK%\.cicada\schematic.cicada_sch" --lib-dir "%ROOT%\assets\cicada-libs" --user-lib-dir "%USERLIB%" --log "%LOGDIR%\accept-engine.txt"
set LINE=
for /l %%i in (1,1,40) do (
  timeout /t 1 /nobreak >nul
  for /f "usebackq tokens=* delims=" %%a in ("%LOGDIR%\accept-engine.txt") do set LINE=%%a
  echo !LINE! | findstr /c:"cicada-engine:" >nul && goto :ready
)
echo ACCEPT-FAIL: no engine announce
taskkill /F /IM cicada-engine.exe >nul 2>&1
exit /b 1
:ready
for /f "tokens=3,4 delims=: " %%a in ("!LINE!") do set PORT=%%a& set TOKEN=%%b
echo engine at 127.0.0.1:!PORT!

rem shape block (prod example) into the workspace auto-resolve location
copy /y "%ROOT%\datasheet-example\AMS1117-3.3\shape.json" "%WORK%\datasheet\AMS1117\shape.json" >nul

rem API key silently (never echo)
for /f "tokens=1* delims=:" %%a in ('findstr /b "key:" "%ROOT%\API-Key.txt"') do set APIKEY=%%b
for /f "tokens=1* delims=:" %%a in ('findstr /b "endpoint" "%ROOT%\API-Key.txt"') do set BASEURL=%%b

rem headless session: main -> spawn_producer -> place/connect/power -> commit
cd /d "%WORK%"
set DSH_HOME=%HOME%
set CICADA_ENGINE_URL=http://127.0.0.1:!PORT!
set CICADA_ENGINE_TOKEN=!TOKEN!
set CICADA_LIB_DIR=%ROOT%\assets\cicada-libs
set CICADA_USER_LIB_DIR=%USERLIB%
set DEEPSEEK_API_KEY=!APIKEY!
set DEEPSEEK_BASE_URL=!BASEURL!
echo step-before-node
echo step-node-begin
node --import tsx/esm C:\dsh\Schematic-Cicada\cicada-harness\apps\cli\src\bin.ts --profile cicada-headless "Design an AMS1117-3.3 fixed 3.3V linear regulator circuit. Use the spawn_producer tool and wait for completion. Producer prompt: Place U1=AMS1117-3.3 (lib_id IC:AMS1117), C1=10uF (lib_id C:C), C2=22uF (lib_id C:C). Then connect endpoints [U1.3-C1.1, U1.2-C2.1, U1.1-C1.2, C1.2-C2.2]. Then place power symbols +5V at U1.3, GND at C1.2, GND at C2.2. Report once placed." > C:\dsh\Schematic-Cicada\cicada-editor\build-msvc-kicad\accept-session.log 2>&1
echo session exit=%ERRORLEVEL%
cd /d "%ROOT%"

rem export from engine state + KiCad official parser validation
curl -s -m 5 -X POST -H "X-Cicada-Token: !TOKEN!" -d "{\"path\":\"%WORK%\\.cicada\\accept.kicad_sch\"}" http://127.0.0.1:!PORT!/export > "%LOGDIR%\accept-export.json"
%KICAD% sch export netlist --output "%WORK%\accept.net" "%WORK%\.cicada\accept.kicad_sch" > "%LOGDIR%\accept-kicad.txt" 2>&1
if errorlevel 1 (
  echo KICAD-FAIL
  type "%LOGDIR%\accept-kicad.txt"
  taskkill /F /IM cicada-engine.exe >nul 2>&1
  exit /b 1
)
echo KICAD-OK
curl -s -m 5 -X POST -H "X-Cicada-Token: !TOKEN!" http://127.0.0.1:!PORT!/shutdown >nul
timeout /t 2 /nobreak >nul
taskkill /F /IM cicada-engine.exe >nul 2>&1
echo ACCEPT-DONE
