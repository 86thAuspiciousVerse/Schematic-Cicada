@echo off
rem M1a 冒烟：cicada-engine 全链路（scene/ops/shutdown），临时副本不改 fixture。
setlocal EnableDelayedExpansion
set ROOT=C:\dsh\Schematic-Cicada
set EXE=%ROOT%\cicada-editor\build-msvc-kicad\Release\cicada-engine.exe
set WORK=%ROOT%\cicada-editor\build-msvc-kicad\smoke-work
set FIX=%~1
if "%FIX%"=="" set FIX=%ROOT%\cicada-harness\packages\cicada\cicada-format\tests\fixtures\minimal.cicada_sch
if exist "%WORK%" rd /s /q "%WORK%"
mkdir "%WORK%"
copy /y "%FIX%" "%WORK%\smoke.cicada_sch" >nul
start /b "" "%EXE%" --port 0 --file "%WORK%\smoke.cicada_sch" --log "%WORK%\engine.txt"
set LINE=
for /l %%i in (1,1,40) do (
  timeout /t 1 /nobreak >nul
  for /f "usebackq tokens=* delims=" %%a in ("%WORK%\engine.txt") do set LINE=%%a
  echo !LINE! | findstr /c:"cicada-engine:" >nul && goto :ready
)
echo SMOKE-FAIL: no announce line
taskkill /F /IM cicada-engine.exe >nul 2>&1
exit /b 1
:ready
for /f "tokens=3,4 delims=: " %%a in ("!LINE!") do set PORT=%%a& set TOKEN=%%b
echo engine at 127.0.0.1:!PORT!
curl -s -m 5 -H "X-Cicada-Token: !TOKEN!" http://127.0.0.1:!PORT!/scene > "%WORK%\scene.json"
findstr /c:"\"components\":" "%WORK%\scene.json" >nul || (echo SMOKE-FAIL: /scene & goto :kill)
curl -s -m 5 -X POST -H "X-Cicada-Token: !TOKEN!" -d "{\"fileHash\":\"\",\"op\":\"draw-wire\",\"points\":[[1000,2000],[3000,2000],[5000,4000]]}" http://127.0.0.1:!PORT!/ops > "%WORK%\ops.json"
findstr /c:"\"ok\":true" "%WORK%\ops.json" >nul || (echo SMOKE-FAIL: /ops & goto :kill)
rem /ops 后重新抓 scene（此前抓的是操作前快照——首顶点断言必须基于操作后）
curl -s -m 5 -H "X-Cicada-Token: !TOKEN!" http://127.0.0.1:!PORT!/scene > "%WORK%\scene.json"
findstr /c:"1000,2000" "%WORK%\scene.json" >nul || (echo SMOKE-FAIL: /ops first vertex lost & goto :kill)
curl -s -m 5 -X POST -H "X-Cicada-Token: !TOKEN!" http://127.0.0.1:!PORT!/shutdown >nul
timeout /t 2 /nobreak >nul
taskkill /F /IM cicada-engine.exe >nul 2>&1
echo SMOKE-OK
exit /b 0
:kill
taskkill /F /IM cicada-engine.exe >nul 2>&1
exit /b 1
