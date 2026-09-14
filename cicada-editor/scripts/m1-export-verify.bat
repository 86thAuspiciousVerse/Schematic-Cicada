@echo off
rem M1 acceptance export + round-trip probe (docs/06 sec.3): runs the real demo
rem document through /export -> kicad-cli netlist -> reload -> scene compare.
rem usage: m1-export-verify.bat [workspace-dir]   (default: the M1 demo workspace)
setlocal
set ROOT=C:\dsh\Schematic-Cicada
set WS=%~1
if "%WS%"=="" set WS=C:\dsh\cicada-m1accept
set SRC=%WS%\.cicada\schematic.cicada_sch
set OUT=%WS%\.cicada\export.kicad_sch
set KICAD=C:\Program Files\KiCad\10.0\bin\kicad-cli.exe
if not exist "%SRC%" (
  echo EXPORT-VERIFY-FAIL: no schematic at %SRC%
  exit /b 1
)
"C:\Program Files\nodejs\node.exe" "%ROOT%\cicada-editor\scripts\m1-export-verify.mjs" ^
  "%ROOT%\cicada-editor\build-msvc-kicad\Release\cicada-engine.exe" ^
  "%ROOT%\assets\cicada-libs" ^
  "%ROOT%\cicada-dev-home\cicada-user-lib" ^
  "%SRC%" ^
  "%OUT%" ^
  "%KICAD%"
exit /b %ERRORLEVEL%
