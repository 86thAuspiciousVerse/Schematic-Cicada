@echo off
rem Debug-config helper for kicad-model-slice3-b3-test (PDB symbols for stack dumps).
set CMAKE=C:\Program Files\CMake\bin\cmake.exe
"%CMAKE%" --build C:\dsh\Schematic-Cicada\cicada-editor\build-msvc-kicad --config Debug --target kicad-model-slice3-b3-test --parallel
exit /b %ERRORLEVEL%
