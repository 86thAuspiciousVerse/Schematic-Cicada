@echo off
rem M1e-1 端到端：/lib/synthesize（形状块→IC:AMS1117 用户库）→ /lib/get → place → 覆盖 → 重启持久化 → 400 拒绝
setlocal EnableDelayedExpansion
set ROOT=C:\dsh\Schematic-Cicada
set EXE=%ROOT%\cicada-editor\build-msvc-kicad\Release\cicada-engine.exe
set LIB=%ROOT%\assets\cicada-libs
set WORK=%ROOT%\cicada-editor\build-msvc-kicad\m1e-synth
if exist "%WORK%" rd /s /q "%WORK%"
mkdir "%WORK%"
mkdir "%WORK%\user-lib"
copy /y "%ROOT%\cicada-harness\packages\cicada\cicada-format\tests\fixtures\minimal.cicada_sch" "%WORK%\doc.cicada_sch" >nul
taskkill /F /IM cicada-engine.exe >nul 2>&1
start /b "" "%EXE%" --port 0 --file "%WORK%\doc.cicada_sch" --lib-dir "%LIB%" --user-lib-dir "%WORK%\user-lib" --log "%WORK%\engine.txt"
set LINE=
for /l %%i in (1,1,40) do (
  timeout /t 1 /nobreak >nul
  for /f "usebackq tokens=* delims=" %%a in ("%WORK%\engine.txt") do set LINE=%%a
  echo !LINE! | findstr /c:"cicada-engine:" >nul && goto :ready
)
echo SYNTH-FAIL: no announce
taskkill /F /IM cicada-engine.exe >nul 2>&1
exit /b 1
:ready
for /f "tokens=3,4 delims=: " %%a in ("!LINE!") do set PORT=%%a& set TOKEN=%%b
rem 1) 合成（AMS1117：1 左 / 2-3 右）
curl -s -m 5 -X POST -H "X-Cicada-Token: !TOKEN!" -d "{\"name\":\"AMS1117\",\"refPrefix\":\"U\",\"description\":\"1A LDO\",\"pins\":[{\"number\":\"1\",\"name\":\"GND\",\"electrical\":\"power_in\",\"side\":\"left\"},{\"number\":\"2\",\"name\":\"VOUT\",\"electrical\":\"power_out\",\"side\":\"right\"},{\"number\":\"3\",\"name\":\"VIN\",\"electrical\":\"power_in\",\"side\":\"right\"}]}" http://127.0.0.1:!PORT!/lib/synthesize > "%WORK%\s1.json"
rem 2) 同键覆盖 → warnings replaced
curl -s -m 5 -X POST -H "X-Cicada-Token: !TOKEN!" -d "{\"name\":\"AMS1117\",\"refPrefix\":\"U\",\"pins\":[{\"number\":\"1\",\"name\":\"GND\",\"electrical\":\"power_in\",\"side\":\"left\"},{\"number\":\"2\",\"name\":\"VOUT\",\"electrical\":\"power_out\",\"side\":\"right\"},{\"number\":\"3\",\"name\":\"VIN\",\"electrical\":\"power_in\",\"side\":\"right\"}]}" http://127.0.0.1:!PORT!/lib/synthesize > "%WORK%\s2.json"
rem 3) 非法 electrical → 400
curl -s -m 5 -X POST -H "X-Cicada-Token: !TOKEN!" -d "{\"name\":\"BAD\",\"pins\":[{\"number\":\"1\",\"name\":\"X\",\"electrical\":\"mystery\"}]}" http://127.0.0.1:!PORT!/lib/synthesize > "%WORK%\s3.json"
rem 3b) 未命中 → 404 not_found（runtime 自动查缺 miss 判定依据；docs/09 §5）
curl -s -m 5 -o "%WORK%\miss.json" -X POST -H "X-Cicada-Token: !TOKEN!" -d "{\"libId\":\"IC:NOPE\"}" http://127.0.0.1:!PORT!/lib/get >nul
rem 4) /lib/get IC:AMS1117（fields + pins IU）
curl -s -m 5 -X POST -H "X-Cicada-Token: !TOKEN!" -d "{\"libId\":\"IC:AMS1117\"}" http://127.0.0.1:!PORT!/lib/get > "%WORK%\get.json"
rem 5) place 到画布（fileHash 空 = 无版本守卫）
curl -s -m 5 -X POST -H "X-Cicada-Token: !TOKEN!" -d "{\"fileHash\":\"\",\"op\":\"place-symbol\",\"libId\":\"IC:AMS1117\",\"x\":25400,\"y\":25400}" http://127.0.0.1:!PORT!/ops > "%WORK%\place.json"
curl -s -m 5 -H "X-Cicada-Token: !TOKEN!" http://127.0.0.1:!PORT!/scene > "%WORK%\scene.json"
rem 6) 用户库文件确实落盘
dir /b "%WORK%\user-lib\IC" > "%WORK%\dirlist.txt"
curl -s -m 5 -X POST -H "X-Cicada-Token: !TOKEN!" http://127.0.0.1:!PORT!/shutdown >nul
timeout /t 2 /nobreak >nul
taskkill /F /IM cicada-engine.exe >nul 2>&1
rem 7) 重启（同一 user-lib）→ 持久化装载
start /b "" "%EXE%" --port 0 --user-lib-dir "%WORK%\user-lib" --log "%WORK%\engine2.txt"
set LINE2=
for /l %%i in (1,1,40) do (
  timeout /t 1 /nobreak >nul
  for /f "usebackq tokens=* delims=" %%a in ("%WORK%\engine2.txt") do set LINE2=%%a
  echo !LINE2! | findstr /c:"cicada-engine:" >nul && goto :ready2
)
echo SYNTH-FAIL: no announce (2nd)
taskkill /F /IM cicada-engine.exe >nul 2>&1
exit /b 1
:ready2
for /f "tokens=3,4 delims=: " %%a in ("!LINE2!") do set PORT2=%%a& set TOKEN2=%%b
curl -s -m 5 -X POST -H "X-Cicada-Token: !TOKEN2!" -d "{\"libId\":\"IC:AMS1117\"}" http://127.0.0.1:!PORT2!/lib/get > "%WORK%\get2.json"
curl -s -m 5 -H "X-Cicada-Token: !TOKEN2!" http://127.0.0.1:!PORT2!/lib/list > "%WORK%\list2.json"
curl -s -m 5 -X POST -H "X-Cicada-Token: !TOKEN2!" http://127.0.0.1:!PORT2!/shutdown >nul
timeout /t 2 /nobreak >nul
taskkill /F /IM cicada-engine.exe >nul 2>&1
node "%ROOT%\cicada-editor\scripts\m1e-synth-view.mjs" "%WORK%"
