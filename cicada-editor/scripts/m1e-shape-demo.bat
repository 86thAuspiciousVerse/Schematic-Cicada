@echo off
rem M1e-1 演示链：用 datasheet subagent 产出的形状块（datasheet-example/AMS1117-3.3/shape.json）
rem → 引擎 /lib/synthesize → /lib/get IC:AMS1117（用户库 + 几何引擎生成的证据）
setlocal EnableDelayedExpansion
set ROOT=C:\dsh\Schematic-Cicada
set EXE=%ROOT%\cicada-editor\build-msvc-kicad\Release\cicada-engine.exe
set WORK=%ROOT%\cicada-editor\build-msvc-kicad\m1e-shape-demo
if exist "%WORK%" rd /s /q "%WORK%"
mkdir "%WORK%\user-lib"
taskkill /F /IM cicada-engine.exe >nul 2>&1
start /b "" "%EXE%" --port 0 --user-lib-dir "%WORK%\user-lib" --log "%WORK%\engine.txt"
set LINE=
for /l %%i in (1,1,40) do (
  timeout /t 1 /nobreak >nul
  for /f "usebackq tokens=* delims=" %%a in ("%WORK%\engine.txt") do set LINE=%%a
  echo !LINE! | findstr /c:"cicada-engine:" >nul && goto :ready
)
echo DEMO-FAIL: no announce
taskkill /F /IM cicada-engine.exe >nul 2>&1
exit /b 1
:ready
for /f "tokens=3,4 delims=: " %%a in ("!LINE!") do set PORT=%%a& set TOKEN=%%b
node "%ROOT%\cicada-editor\scripts\m1e-shape-demo.mjs" "%PORT%" "%TOKEN%" "%WORK%" "%ROOT%\datasheet-example\AMS1117-3.3\shape.json"
curl -s -m 5 -X POST -H "X-Cicada-Token: !TOKEN!" http://127.0.0.1:!PORT!/shutdown >nul
timeout /t 2 /nobreak >nul
taskkill /F /IM cicada-engine.exe >nul 2>&1
