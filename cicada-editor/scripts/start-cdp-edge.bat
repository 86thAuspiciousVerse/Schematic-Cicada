@echo off
rem M1c verify: headless Edge with CDP port for m1c-cdp.mjs (fresh profile).
set OUT=C:\dsh\Schematic-Cicada\cicada-editor\build-msvc-kicad
set PROF=%OUT%\cdp-profile
if exist "%PROF%" rmdir /s /q "%PROF%"
start "" "C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe" --headless=new --disable-gpu --no-first-run --disable-extensions --disable-sync --user-data-dir="%PROF%" --remote-debugging-port=9333 --window-size=1500,950 about:blank
echo CDP-EDGE-LAUNCHED
