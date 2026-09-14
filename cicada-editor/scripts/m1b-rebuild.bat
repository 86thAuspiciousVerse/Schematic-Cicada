@echo off
rem M1b 收尾重编译：杀引擎/编译进程 → 删改过文件 obj + PDB → 顺序构建引擎与 libtest。
rem 严禁与其它构建并发（PDB 冲突 C1041 / 孤儿 cl.exe）。
setlocal
set ROOT=C:\dsh\Schematic-Cicada
set SRC=%ROOT%\cicada-editor
set BUILD=%ROOT%\cicada-editor\build-msvc-kicad

taskkill /F /IM cicada-engine.exe >nul 2>&1
taskkill /F /IM cl.exe >nul 2>&1
taskkill /F /IM MSBuild.exe >nul 2>&1

del /q "%BUILD%\cicada-engine.dir\Release\cicada_document_bridge.obj" >nul 2>&1
del /q "%BUILD%\cicada-engine.dir\Release\main_service.obj" >nul 2>&1
del /q "%BUILD%\cicada-engine.dir\Release\ops.obj" >nul 2>&1
del /q "%BUILD%\cicada-engine.dir\Release\scene_json.obj" >nul 2>&1
del /q "%BUILD%\cicada-engine-libtest.dir\Release\cicada_document_bridge.obj" >nul 2>&1
del /q "%BUILD%\cicada-engine-libtest.dir\Release\lib_roundtrip_test.obj" >nul 2>&1
del /q "%BUILD%\cicada-engine.dir\Release\vc143.pdb" >nul 2>&1
del /q "%BUILD%\cicada-engine-libtest.dir\Release\vc143.pdb" >nul 2>&1

call "%SRC%\scripts\k6-reload-build.bat" cicada-engine || exit /b 1
call "%SRC%\scripts\k6-reload-build.bat" cicada-engine-libtest || exit /b 1
call "%SRC%\scripts\k6-reload-build.bat" cicada-engine-shapesynth-test || exit /b 1
echo M1B-REBUILD-OK
