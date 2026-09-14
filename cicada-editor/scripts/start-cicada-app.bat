@echo off
rem M1d: product launcher (docs/04 §1). Engine binary & lib dir via env.
setlocal
set DSH_HOME=C:\dsh\Schematic-Cicada\cicada-dev-home
set CICADA_HOME=C:\dsh\Schematic-Cicada\cicada-dev-home
if not defined CICADA_ENGINE_EXE set CICADA_ENGINE_EXE=C:\dsh\Schematic-Cicada\cicada-editor\build-msvc-kicad\Release\cicada-engine.exe
rem M1b 精选库（60 枚）；M1e-1 用户库（/lib/synthesize 落盘，跨会话持久）
if not defined CICADA_LIB_DIR set CICADA_LIB_DIR=C:\dsh\Schematic-Cicada\assets\cicada-libs
if not defined CICADA_USER_LIB_DIR set CICADA_USER_LIB_DIR=C:\dsh\Schematic-Cicada\cicada-dev-home\cicada-user-lib
rem MinerU token (product channel = settings.yaml, this is the env fallback).
if not defined MINERU_TOKEN if exist "C:\dsh\Schematic-Cicada\MinerU\api_token.md" for /f "usebackq delims=" %%t in (`""C:\Program Files\nodejs\node.exe" "C:\dsh\Schematic-Cicada\cicada-editor\scripts\mineru-token.mjs" "C:\dsh\Schematic-Cicada\MinerU\api_token.md""`) do set "MINERU_TOKEN=%%t"
rem Optional Tavily MCP: read the key from the gitignored credential file into an
rem env var (the cordis row is disabled when it is absent). Never from source.
if not defined TAVILY_API_KEY if exist "C:\dsh\Schematic-Cicada\tavily-mcp.txt" for /f "usebackq delims=" %%t in (`""C:\Program Files\nodejs\node.exe" "C:\dsh\Schematic-Cicada\cicada-editor\scripts\tavily-key.mjs" "C:\dsh\Schematic-Cicada\tavily-mcp.txt""`) do set "TAVILY_API_KEY=%%t"
rem product brand icon (page favicon / Edge --app window icon via editor-bridge)
if not defined CICADA_BRAND_ICON set CICADA_BRAND_ICON=C:\dsh\Schematic-Cicada\assets\brand\cicada-64.png
if not defined CICADA_BRAND_MARK set CICADA_BRAND_MARK=C:\dsh\Schematic-Cicada\assets\brand\cicada-mark.png
cd /d C:\dsh\Schematic-Cicada\cicada-harness
"C:\Program Files\nodejs\node.exe" --import tsx/esm "C:\dsh\Schematic-Cicada\cicada-harness\packages\cicada\cicada-launcher\bin\cicada-app.ts" %*
