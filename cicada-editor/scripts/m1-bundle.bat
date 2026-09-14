@echo off
rem M1 client bundle rebuild (canvas/layout client packages).
rem The host registry serves lib\client.js from disk: rebuild, then refresh the page
rem (or run `pnpm run dev:web` for HMR). tsdown is invoked directly to bypass the
rem pnpm deps-status check, which tries to reinstall across the WSL/Windows split.
rem usage: m1-bundle.bat [package-dir-name]   (default ui-cicada-canvas)
setlocal
set ROOT=C:\dsh\Schematic-Cicada
set LOGDIR=%ROOT%\cicada-editor\logs
if not exist "%LOGDIR%" mkdir "%LOGDIR%"
set PKG=%~1
if "%PKG%"=="" set PKG=ui-cicada-canvas
cd /d %ROOT%\cicada-harness\packages\client\%PKG%
"C:\Program Files\nodejs\node.exe" "%ROOT%\cicada-harness\node_modules\tsdown\dist\run.mjs" > "%LOGDIR%\bundle-%PKG%.log" 2>&1
set RC=%ERRORLEVEL%
echo bundle %PKG% exit=%RC%
type "%LOGDIR%\bundle-%PKG%.log"
exit /b %RC%
