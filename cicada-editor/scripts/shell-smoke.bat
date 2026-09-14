@echo off
rem Shell headless smoke (docs/04 sec 2 / docs/06 sec 3): no window, no tray.
setlocal
cd /d "%~dp0..\..\cicada-shell"
if not exist logs mkdir logs
node_modules\electron\dist\electron.exe . --smoke=logs\shell-smoke.json
set RC=%ERRORLEVEL%
type logs\shell-smoke.json
echo [shell-smoke] exit=%RC%
exit /b %RC%
