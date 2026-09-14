@echo off
rem M1c verify: headless Edge screenshot of the workspace layout (fresh profile per run).
set OUT=C:\dsh\Schematic-Cicada\cicada-editor\build-msvc-kicad
set PROF=%OUT%\shot-profile
set /p WEB=<%OUT%\web-url.txt
if "%WEB%"=="" echo NO-WEB-URL && exit /b 1
if exist "%PROF%" rmdir /s /q "%PROF%"
echo USING=[%WEB%] >> "%OUT%\m1c-dom.txt"
"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe" --headless=new --disable-gpu --no-first-run --disable-extensions --disable-sync --user-data-dir="%PROF%" --window-size=1500,950 --virtual-time-budget=12000 --screenshot="%OUT%\m1c-shot.png" "%WEB%&layout=workspace" >> "%OUT%\m1c-dom.txt" 2>&1
echo SHOT_EXIT=%ERRORLEVEL%
