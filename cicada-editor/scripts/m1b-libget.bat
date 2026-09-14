@echo off
rem M1b 收尾复验：空文档引擎 + --lib-dir → /lib/list（键唯一性）+ /lib/get R:R_Shunt（fields 五元）。
setlocal EnableDelayedExpansion
set ROOT=C:\dsh\Schematic-Cicada
set EXE=%ROOT%\cicada-editor\build-msvc-kicad\Release\cicada-engine.exe
set LIB=%ROOT%\assets\cicada-libs
set WORK=%ROOT%\cicada-editor\build-msvc-kicad\m1b-libget
if exist "%WORK%" rd /s /q "%WORK%"
mkdir "%WORK%"
taskkill /F /IM cicada-engine.exe >nul 2>&1
start /b "" "%EXE%" --port 0 --lib-dir "%LIB%" --log "%WORK%\engine.txt"
set LINE=
for /l %%i in (1,1,40) do (
  timeout /t 1 /nobreak >nul
  for /f "usebackq tokens=* delims=" %%a in ("%WORK%\engine.txt") do set LINE=%%a
  echo !LINE! | findstr /c:"cicada-engine:" >nul && goto :ready
)
echo M1B-LIBGET-FAIL: no announce line
taskkill /F /IM cicada-engine.exe >nul 2>&1
exit /b 1
:ready
for /f "tokens=3,4 delims=: " %%a in ("!LINE!") do set PORT=%%a& set TOKEN=%%b
echo engine at 127.0.0.1:!PORT!
curl -s -m 5 -H "X-Cicada-Token: !TOKEN!" http://127.0.0.1:!PORT!/lib/list > "%WORK%\list.json"
curl -s -m 5 -X POST -H "X-Cicada-Token: !TOKEN!" -d "{\"libId\":\"R:R_Shunt\"}" http://127.0.0.1:!PORT!/lib/get > "%WORK%\get.json"
curl -s -m 5 -X POST -H "X-Cicada-Token: !TOKEN!" -d "{\"libId\":\"LED:LED\"}" http://127.0.0.1:!PORT!/lib/get > "%WORK%\get-led.json"
curl -s -m 5 -X POST -H "X-Cicada-Token: !TOKEN!" http://127.0.0.1:!PORT!/shutdown >nul
timeout /t 2 /nobreak >nul
taskkill /F /IM cicada-engine.exe >nul 2>&1
echo === 校验 ===
node "%ROOT%\cicada-editor\scripts\m1b-libget-check.mjs" "%WORK%"
