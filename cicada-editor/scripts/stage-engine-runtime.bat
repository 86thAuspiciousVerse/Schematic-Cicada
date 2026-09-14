@echo off
rem App-local VC++ runtime staging for cicada-engine.exe (docs/04 sec 2.5).
rem Locates the VC143 CRT redist through vswhere (no machine literals) and copies
rem it next to the engine binary, so the product needs neither a redist install
rem nor admin rights. Usage: stage-engine-runtime.bat [engine output dir]
rem Every expansion of a variable that may hold "(x86)" stays quoted: an
rem unquoted parenthesis inside a command block breaks the cmd parser.
setlocal enabledelayedexpansion
set "HERE=%~dp0"
set "OUTDIR=%~1"
if "%OUTDIR%"=="" set "OUTDIR=%HERE%..\build-msvc-kicad\Release"
set "LOGDIR=%HERE%..\logs"
if not exist "%LOGDIR%" mkdir "%LOGDIR%"
set "LOG=%LOGDIR%\stage-engine-runtime.log"

if not exist "%OUTDIR%\cicada-engine.exe" (
  echo [stage] engine binary not found: "%OUTDIR%\cicada-engine.exe"
  echo [stage] engine binary not found > "%LOG%"
  exit /b 1
)

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
  echo [stage] vswhere not found: "%VSWHERE%"
  echo [stage] vswhere not found > "%LOG%"
  exit /b 1
)
set "VSPATH="
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -property installationPath`) do set "VSPATH=%%i"
if "%VSPATH%"=="" (
  echo [stage] Visual Studio installation not found
  echo [stage] Visual Studio installation not found > "%LOG%"
  exit /b 1
)

rem Newest version directory that actually carries the x64 VC143 CRT.
set "CRTDIR="
for /f "delims=" %%d in ('dir /b /ad /o-n "%VSPATH%\VC\Redist\MSVC" 2^>nul') do (
  if not defined CRTDIR if exist "%VSPATH%\VC\Redist\MSVC\%%d\x64\Microsoft.VC143.CRT\msvcp140.dll" set "CRTDIR=%VSPATH%\VC\Redist\MSVC\%%d\x64\Microsoft.VC143.CRT"
)
if not defined CRTDIR (
  echo [stage] VC143 CRT redist not found under "%VSPATH%\VC\Redist\MSVC"
  echo [stage] VC143 CRT redist not found > "%LOG%"
  exit /b 1
)

set COUNT=0
for %%f in ("%CRTDIR%\*.dll") do (
  copy /Y "%%f" "%OUTDIR%\" >nul
  if errorlevel 1 (
    echo [stage] copy failed: "%%f"
    echo [stage] copy failed >> "%LOG%"
    exit /b 1
  )
  set /a COUNT+=1
)
echo [stage] copied !COUNT! dll^(s^) from "%CRTDIR%"
echo [stage] copied !COUNT! dll^(s^) to "%OUTDIR%" > "%LOG%"
exit /b 0
