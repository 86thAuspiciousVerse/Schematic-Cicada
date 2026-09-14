@echo off
rem Shell unit tests (docs/04 sec 2): plain node --test, no build step.
setlocal
cd /d "%~dp0..\..\cicada-shell"
if not exist logs mkdir logs
node --test tests\config.test.mjs tests\status.test.mjs tests\stack.test.mjs tests\projects.test.mjs tests\syntax.test.mjs tests\manager-core.test.mjs tests\manager-client.test.mjs tests\manager-symbols.test.mjs tests\engine-client.test.mjs tests\launcher-ui.test.mjs tests\ui-smoke.test.mjs > logs\shell-test.log 2>&1
set RC=%ERRORLEVEL%
type logs\shell-test.log
echo [shell-test] exit=%RC%
exit /b %RC%
