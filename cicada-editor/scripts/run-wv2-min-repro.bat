@echo off
cd /d C:\dsh\Schematic-Cicada\cicada-editor
echo === VARIANT 2: Edge stable folder ===
build-msvc-kicad\Release\wv2_min_repro.exe "C:\Program Files (x86)\Microsoft\Edge\Application\152.0.4191.62" > logs\wv2-min-repro-edge.txt 2>&1
echo V2_EXIT=%ERRORLEVEL%
echo === VARIANT 1 rerun for exit code ===
build-msvc-kicad\Release\wv2_min_repro.exe > logs\wv2-min-repro.txt 2>&1
echo V1_EXIT=%ERRORLEVEL%
