@echo off
rem Offline Electron binary install (docs/04 sec 2). Put the downloaded zip here
rem or pass its path: install-electron-offline.bat <zip>
setlocal
cd /d "%~dp0..\..\cicada-shell"
if "%~1"=="" (
  echo usage: install-electron-offline.bat ^<electron-v^<version^>-win32-x64.zip^>
  exit /b 2
)
node tools\install-electron-offline.mjs "%~1"
exit /b %ERRORLEVEL%
