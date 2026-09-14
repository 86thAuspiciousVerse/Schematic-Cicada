@echo off
rem M1e 探针：place R:R_Shunt → /scene pins 坐标量级（IU vs G 尺度确认）
setlocal EnableDelayedExpansion
set ROOT=C:\dsh\Schematic-Cicada
set EXE=%ROOT%\cicada-editor\build-msvc-kicad\Release\cicada-engine.exe
set LIB=%ROOT%\assets\cicada-libs
set WORK=%ROOT%\cicada-editor\build-msvc-kicad\m1e-probe
if exist "%WORK%" rd /s /q "%WORK%"
mkdir "%WORK%"
copy /y "%ROOT%\cicada-harness\packages\cicada\cicada-format\tests\fixtures\minimal.cicada_sch" "%WORK%\probe.cicada_sch" >nul
taskkill /F /IM cicada-engine.exe >nul 2>&1
start /b "" "%EXE%" --port 0 --file "%WORK%\probe.cicada_sch" --lib-dir "%LIB%" --log "%WORK%\engine.txt"
set LINE=
for /l %%i in (1,1,40) do (
  timeout /t 1 /nobreak >nul
  for /f "usebackq tokens=* delims=" %%a in ("%WORK%\engine.txt") do set LINE=%%a
  echo !LINE! | findstr /c:"cicada-engine:" >nul && goto :ready
)
echo PROBE-FAIL: no announce
taskkill /F /IM cicada-engine.exe >nul 2>&1
exit /b 1
:ready
for /f "tokens=3,4 delims=: " %%a in ("!LINE!") do set PORT=%%a& set TOKEN=%%b
curl -s -m 5 -X POST -H "X-Cicada-Token: !TOKEN!" -d "{\"fileHash\":\"\",\"op\":\"place-symbol\",\"libId\":\"R:R_Shunt\",\"x\":10000,\"y\":10000}" http://127.0.0.1:!PORT!/ops > "%WORK%\ops.json"
curl -s -m 5 -H "X-Cicada-Token: !TOKEN!" http://127.0.0.1:!PORT!/scene > "%WORK%\scene.json"
node "%ROOT%\cicada-editor\scripts\m1e-probe-view.mjs" "%WORK%"
curl -s -m 5 -X POST -H "X-Cicada-Token: !TOKEN!" http://127.0.0.1:!PORT!/shutdown >nul
timeout /t 2 /nobreak >nul
taskkill /F /IM cicada-engine.exe >nul 2>&1
