@echo off
rem Typecheck the cicada domain packages (Windows side, same reason as m1-vitest).
setlocal
set ROOT=C:\dsh\Schematic-Cicada
cd /d %ROOT%\cicada-harness
"C:\Program Files\nodejs\node.exe" node_modules\typescript\bin\tsc -b ^
  packages/cicada/cicada-format/tsconfig.json ^
  packages/cicada/cicada-runtime/tsconfig.json ^
  packages/cicada/cicada-knowledge/tsconfig.json ^
  packages/cicada/cicada-editor-bridge/tsconfig.json > "%ROOT%\cicada-editor\logs\m1-tsc.log" 2>&1
set RC=%ERRORLEVEL%
echo tsc exit=%RC%
if not "%RC%"=="0" type "%ROOT%\cicada-editor\logs\m1-tsc.log"
exit /b %RC%
