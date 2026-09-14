@echo off
rem lib_symbols regression (cases A-F): key fidelity, cross-family tolerance, export body.
rem usage: m1b-libtest.bat   (fixture + out dir are fixed here; see docs/06 sec.3)
setlocal
set ROOT=C:\dsh\Schematic-Cicada
set OUT=%ROOT%\cicada-editor\logs\libtest-out
if not exist "%OUT%" mkdir "%OUT%"
"%ROOT%\cicada-editor\build-msvc-kicad\Release\cicada-engine-libtest.exe" ^
  "%ROOT%\cicada-harness\packages\cicada\cicada-format\tests\fixtures\minimal.cicada_sch" ^
  "%OUT%" > "%ROOT%\cicada-editor\logs\m1b-libtest.log" 2>&1
set RC=%ERRORLEVEL%
type "%ROOT%\cicada-editor\logs\m1b-libtest.log"
if not "%RC%"=="0" echo LIBTEST-FAIL (%RC%)
if "%RC%"=="0" echo LIBTEST-OK
exit /b %RC%
