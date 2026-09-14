@echo off
rem Headless pipeline probe (dev-local launcher): boots the headless profile with the cicada
rem domain rows + Tavily MCP via --patch, the probe DSH_HOME (product model route + MinerU token),
rem and the probe workspace as the session cwd. Task text lives in the workspace .md file.
rem Usage: probe-pipeline-headless.bat [task-file.md] [engine-url] [engine-token]
rem        (default task: probe-task-publish.md)
setlocal
set "TASK=%~1"
if "%TASK%"=="" set "TASK=probe-task-publish.md"
rem The runtime's env fallback client needs CICADA_ENGINE_URL/CICADA_ENGINE_TOKEN, and WSL
rem interop does NOT forward WSL env vars into Windows processes (measured 2026-09-12: a var
rem set on the WSL side arrives unexpanded, WSLENV or not) -- so run-probe.py passes the
rem endpoint as arguments and it is set here, on the Windows side.
if not "%~2"=="" set "CICADA_ENGINE_URL=%~2"
if not "%~3"=="" set "CICADA_ENGINE_TOKEN=%~3"
set DSH_HOME=C:\dsh\Schematic-Cicada\cicada-dev-home\probe-pipeline
rem CICADA_HOME decides where the shared datasheet library lives (resolveDshHome);
rem without it the probe would write into the default ~/.cicada/home instead of the probe home.
set CICADA_HOME=C:\dsh\Schematic-Cicada\cicada-dev-home\probe-pipeline
rem tsx must resolve workspace imports through the harness tsconfig even from another cwd.
set TSX_TSCONFIG_PATH=C:\dsh\Schematic-Cicada\cicada-harness\tsconfig.json
rem The headless profile mounts no settings service, so `mineru.token` in settings.yaml
rem is invisible here: feed the same token through the env var the plugin also accepts,
rem read from the gitignored token file so no secret lives in this tracked script.
if not defined MINERU_TOKEN for /f "usebackq delims=" %%t in ("C:\dsh\Schematic-Cicada\MinerU\api_token.md") do set "MINERU_TOKEN=%%t"
rem Optional 4th argument = workspace directory (the headless session's cwd, and where
rem the task file is read from). Defaults to probe-ws.
if not "%~4"=="" (cd /d "%~4") else (cd /d C:\dsh\Schematic-Cicada\cicada-dev-home\probe-ws)
"C:\Program Files\nodejs\node.exe" --import file:///C:/dsh/Schematic-Cicada/cicada-harness/node_modules/tsx/dist/esm/index.mjs C:\dsh\Schematic-Cicada\cicada-harness\apps\cli\src\bin.ts --profile headless --patch C:\dsh\Schematic-Cicada\cicada-dev-home\probe-pipeline\probe.patch.yml "Read %TASK% in the current directory and follow it exactly." > C:\dsh\Schematic-Cicada\cicada-editor\logs\probe-headless.log 2>&1
echo exit=%ERRORLEVEL% >> C:\dsh\Schematic-Cicada\cicada-editor\logs\probe-headless.log
