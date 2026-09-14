@echo off
rem Slice-1 build helper: reconfigure (refresh GLOB) + model lib + DoD test (Release).
rem Usage: build-slice1.bat [target]
setlocal
set CMAKE=C:\Program Files\CMake\bin\cmake.exe
set ROOT=C:\dsh\Schematic-Cicada
set SRC=%ROOT%\cicada-editor
set BUILD=%ROOT%\cicada-editor\build-msvc-kicad
set TARGET=%~1
if "%TARGET%"=="" set TARGET=kicad-model-slice1-test;kicad-model-slice2-test;kicad-model-slice2-geometry-test;kicad-model-slice3-test;kicad-model-slice3-b3-test
rem Always reconfigure: the model source list is a GLOB over the vendored tree,
rem so newly copied sources must re-enter the vcxproj.
"%CMAKE%" -S "%SRC%" -B "%BUILD%" -G "Visual Studio 17 2022" -A x64 -DCICADA_EDITOR_ENABLE_KICAD_MODEL=ON -DCICADA_EDITOR_ENABLE_CANVAS=ON -DCICADA_EDITOR_BUILD_TESTS=ON -DwxWidgets_ROOT_DIR=%ROOT%\cicada-editor\third_party\wxWidgets -DwxWidgets_LIB_DIR=%ROOT%\cicada-editor\third_party\wxWidgets\build-cicada-msvc\lib\vc_x64_lib -DwxWidgets_CONFIGURATION=mswu >NUL
"%CMAKE%" --build "%BUILD%" --config Release --target %TARGET% --parallel
exit /b %ERRORLEVEL%
