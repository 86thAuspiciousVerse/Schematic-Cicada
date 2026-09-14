@echo off
rem K6：WebView2 嵌入已修复（专用 STA 线程隔离 wx IsDialogMessage 路径）——默认启用。
set CICADA_EDITOR_WEBVIEW2=1
set CICADA_DSH_DEBUG=1
set CICADA_DSH_NODE=C:\Program Files\nodejs\node.exe
set CICADA_DSH_ENTRY=C:\dsh\Schematic-Cicada\cicada-harness\apps\cli\src\bin.ts
set CICADA_DSH_PRELOAD=tsx/esm
set CICADA_DSH_HOME=C:\dsh\Schematic-Cicada\cicada-dev-home
set CICADA_DSH_CWD=C:\dsh\Schematic-Cicada\cicada-harness
start "" "C:\dsh\Schematic-Cicada\cicada-editor\build-msvc-kicad\Release\cicada-editor.exe"
