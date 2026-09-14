@echo off
echo --- cmake ---
"C:\Program Files\CMake\bin\cmake.exe" --version 2>nul | findstr /i "version"
echo --- pnpm ---
call pnpm -v 2>nul
echo --- VS-vcvars ---
dir /b "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build" 2>nul | findstr /i "vcvars64"
echo --- codegraph-index ---
dir /a-d /b C:\dsh\Schematic-Cicada\kicad\.codegraph 2>nul
