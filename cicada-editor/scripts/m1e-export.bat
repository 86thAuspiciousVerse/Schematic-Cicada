@echo off
rem M1 验收链路①：/export（KiCad 10 .kicad_sch）→ kicad-cli sch export netlist（官方解析器验证可打开）
setlocal EnableDelayedExpansion
set ROOT=C:\dsh\Schematic-Cicada
set EXE=%ROOT%\cicada-editor\build-msvc-kicad\Release\cicada-engine.exe
set KICAD="C:\Program Files\KiCad\10.0\bin\kicad-cli.exe"
set WORK=%ROOT%\cicada-editor\build-msvc-kicad\m1e-export
if exist "%WORK%" rd /s /q "%WORK%"
mkdir "%WORK%"
copy /y "%ROOT%\cicada-harness\packages\cicada\cicada-format\tests\fixtures\minimal.cicada_sch" "%WORK%\doc.cicada_sch" >nul
taskkill /F /IM cicada-engine.exe >nul 2>&1
start /b "" "%EXE%" --port 0 --file "%WORK%\doc.cicada_sch" --lib-dir "%ROOT%\assets\cicada-libs" --log "%WORK%\engine.txt"
set LINE=
for /l %%i in (1,1,40) do (
  timeout /t 1 /nobreak >nul
  for /f "usebackq tokens=* delims=" %%a in ("%WORK%\engine.txt") do set LINE=%%a
  echo !LINE! | findstr /c:"cicada-engine:" >nul && goto :ready
)
echo EXPORT-FAIL: no announce
taskkill /F /IM cicada-engine.exe >nul 2>&1
exit /b 1
:ready
for /f "tokens=3,4 delims=: " %%a in ("!LINE!") do set PORT=%%a& set TOKEN=%%b
rem 放一个 R:R_Shunt + 画一条线（让导出有内容）
curl -s -m 5 -X POST -H "X-Cicada-Token: !TOKEN!" -d "{\"fileHash\":\"\",\"op\":\"draw-wire\",\"points\":[[10000,20000],[30000,20000]]}" http://127.0.0.1:!PORT!/ops > "%WORK%\ops.json"
curl -s -m 5 -X POST -H "X-Cicada-Token: !TOKEN!" -d "{\"path\":\"%WORK%\\out.kicad_sch\"}" http://127.0.0.1:!PORT!/export > "%WORK%\export.json"
curl -s -m 5 -X POST -H "X-Cicada-Token: !TOKEN!" http://127.0.0.1:!PORT!/shutdown >nul
timeout /t 2 /nobreak >nul
taskkill /F /IM cicada-engine.exe >nul 2>&1
echo === export.json ===
type "%WORK%\export.json"
echo.
echo === kicad-cli 解析验证 ===
%KICAD% sch export netlist --output "%WORK%\out.net" "%WORK%\out.kicad_sch" > "%WORK%\kicad.txt" 2>&1
if errorlevel 1 (
  echo KICAD-CLI-FAIL
  type "%WORK%\kicad.txt"
  exit /b 1
)
type "%WORK%\kicad.txt"
if exist "%WORK%\out.net" echo KICAD-NETLIST-OK
