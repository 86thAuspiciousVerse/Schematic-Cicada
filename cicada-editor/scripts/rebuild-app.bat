@echo off
rem K6: rebuild cicada-editor only (app target), quiet-ish.
setlocal
set CMAKE=C:\Program Files\CMake\bin\cmake.exe
set BUILD=C:\dsh\Schematic-Cicada\cicada-editor\build-msvc-kicad
"%CMAKE%" --build "%BUILD%" --config Release --target cicada-editor --parallel
exit /b %ERRORLEVEL%
