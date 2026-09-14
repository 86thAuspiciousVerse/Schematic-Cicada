@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
cd /d C:\dsh\Schematic-Cicada\cicada-editor
set VK=src\vendor\kicad
cl /nologo /c /EHsc /std:c++20 /utf-8 /bigobj /Zi /I src /I src\vendor\kicad /I %VK%\include /I %VK%\common /I %VK%\common\dialogs /I %VK%\eeschema /I %VK%\libs\kimath\include /I %VK%\libs\core\include /I %VK%\libs\sexpr\include /I %VK%\libs\kiplatform\include /I %VK%\thirdparty\thread-pool /I %VK%\thirdparty\magic_enum\magic_enum /I %VK%\thirdparty\fast_float\include /I %VK%\thirdparty\dynamic_bitset /I %VK%\thirdparty\rtree /I %VK%\thirdparty\nlohmann_json /I %VK%\thirdparty\fmt\include /I %VK%\thirdparty\clipper2\Clipper2Lib\include /I %VK%\thirdparty\pegtl /I %VK%\thirdparty\expected\include /I %VK%\thirdparty\picosha2 /I %VK%\thirdparty\glm\include /I third_party\boost /I third_party\wxWidgets\build-cicada-msvc\lib\vc_x64_lib\mswu /I third_party\wxWidgets\include /D_USE_MATH_DEFINES=1 /DUSINGZ=1 /DWIN32_LEAN_AND_MEAN=1 /DKICOMMON_DLL=1 /DGAL_DLL=1 /Fo:build\scene_json_check.obj src\service\scene_json.cpp
echo CHECK_EXIT=%ERRORLEVEL%
