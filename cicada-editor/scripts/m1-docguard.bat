@echo off
rem M1 document-guard probe: /ops without a document must be 409, /document must
rem create a missing truth file, and /document "" must clear the document.
rem See m1-docguard.mjs (regression for the 2026-09-08 blank-canvas defect).
setlocal
set ROOT=C:\dsh\Schematic-Cicada
set WORK=%ROOT%\cicada-editor\build-msvc-kicad\m1-docguard
"C:\Program Files\nodejs\node.exe" "%ROOT%\cicada-editor\scripts\m1-docguard.mjs" ^
  "%ROOT%\cicada-editor\build-msvc-kicad\Release\cicada-engine.exe" ^
  "%ROOT%\assets\cicada-libs" ^
  "%ROOT%\cicada-dev-home\cicada-user-lib" ^
  "%WORK%"
exit /b %ERRORLEVEL%
