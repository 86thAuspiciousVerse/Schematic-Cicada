@call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
@cd /d C:\dsh\Schematic-Cicada\cicada-editor\scripts
@cl /nologo /EHsc /Fe:spawn_probe.exe k6_spawn_probe.cpp
