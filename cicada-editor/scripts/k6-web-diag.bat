@echo off
rem K6 web boot diag runner (Windows node, harness root cwd).
setlocal
cd /d C:\dsh\Schematic-Cicada\cicada-harness
"C:\Program Files\nodejs\node.exe" --import tsx/esm C:\dsh\Schematic-Cicada\cicada-editor\scripts\k6-web-diag.mjs
exit /b %ERRORLEVEL%
