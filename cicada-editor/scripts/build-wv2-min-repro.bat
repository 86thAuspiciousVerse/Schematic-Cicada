@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
cd /d C:\dsh\Schematic-Cicada\cicada-editor
cl /nologo /EHsc /std:c++17 /I C:\dsh\Schematic-Cicada\cicada-editor\third_party\webview2\include C:\dsh\Schematic-Cicada\cicada-editor\tests\k6\wv2_min_repro.cpp C:\dsh\Schematic-Cicada\cicada-editor\third_party\webview2\lib\WebView2LoaderStatic.lib ole32.lib user32.lib advapi32.lib /Fe:C:\dsh\Schematic-Cicada\cicada-editor\build-msvc-kicad\Release\wv2_min_repro.exe
echo BUILD_EXIT=%ERRORLEVEL%
