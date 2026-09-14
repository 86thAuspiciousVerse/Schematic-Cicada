@echo off
rem M1 domain tests (packages/cicada + the canvas client package).
rem WSL lacks the rolldown linux binding, so vitest must run on the Windows side:
rem node node_modules\vitest\vitest.mjs run <paths> (see docs/06 sec.3).
setlocal
set ROOT=C:\dsh\Schematic-Cicada
set LOGDIR=%ROOT%\cicada-editor\logs
if not exist "%LOGDIR%" mkdir "%LOGDIR%"
cd /d %ROOT%\cicada-harness
"C:\Program Files\nodejs\node.exe" node_modules\vitest\vitest.mjs run packages/cicada packages/client/ui-cicada-canvas packages/client/ui-cicada-layout > "%LOGDIR%\m1-vitest.log" 2>&1
set RC=%ERRORLEVEL%
echo vitest exit=%RC%
findstr /c:"Test Files" /c:"Tests " "%LOGDIR%\m1-vitest.log"
exit /b %RC%
