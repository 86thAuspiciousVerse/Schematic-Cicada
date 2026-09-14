@echo off
set TK=9b992e0a10f7147d392c18e4161031dd
curl -s -m 8 -X POST -H X-Cicada-Token:%TK% -d "{\"libId\":\"R:R_Shunt\"}" http://127.0.0.1:41149/lib/get -o C:\dsh\Schematic-Cicada\cicada-editor\build-msvc-kicad\libget4.json
