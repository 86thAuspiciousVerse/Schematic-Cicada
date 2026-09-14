@echo off
rem Launcher UI smoke: render every page in a HIDDEN Electron window and read the
rem DOM back (catches real-DOM-only failures the DOM-stub tests cannot see).
rem Report: cicada-shell\logs\shell-ui-smoke.json
setlocal
cd /d "%~dp0..\..\cicada-shell"
if not exist logs mkdir logs
npx --no-install electron . --smoke-ui > logs\shell-ui-smoke.txt 2>&1
set RC=%ERRORLEVEL%
type logs\shell-ui-smoke.txt
echo [shell-ui-smoke] exit=%RC%
exit /b %RC%
