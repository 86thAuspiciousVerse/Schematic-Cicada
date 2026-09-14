@echo off
rem Phase-2 shell (docs/04 sec 2): Electron supervisor (tray + status window).
rem All product paths are derived by the shell from its own location.
setlocal
cd /d "%~dp0..\..\cicada-shell"
if not exist node_modules\electron\dist\electron.exe (
  echo [shell] Electron is not installed. Run: npm install
  pause
  exit /b 1
)
rem MinerU token (product channel = settings.yaml, this is the env fallback).
if not defined MINERU_TOKEN if exist "C:\dsh\Schematic-Cicada\MinerU\api_token.md" for /f "usebackq delims=" %%t in (`""C:\Program Files\nodejs\node.exe" "C:\dsh\Schematic-Cicada\cicada-editor\scripts\mineru-token.mjs" "C:\dsh\Schematic-Cicada\MinerU\api_token.md""`) do set "MINERU_TOKEN=%%t"
rem Optional Tavily MCP: the key is read from the gitignored credential file into
rem an env var (the launcher renders the MCP overlay from it; no key = channel off).
if not defined TAVILY_API_KEY if exist "C:\dsh\Schematic-Cicada\tavily-mcp.txt" for /f "usebackq delims=" %%t in (`""C:\Program Files\nodejs\node.exe" "C:\dsh\Schematic-Cicada\cicada-editor\scripts\tavily-key.mjs" "C:\dsh\Schematic-Cicada\tavily-mcp.txt""`) do set "TAVILY_API_KEY=%%t"

start "" "node_modules\electron\dist\electron.exe" .
exit /b 0
