@echo off
set NODE=C:\Program Files\nodejs\node.exe
set HOST=C:\dsh\Schematic-Cicada\cicada-harness\apps\cli\lib\bin.js
set HOME=C:\dsh\Schematic-Cicada\cicada-dev-home
if "%~1"=="node"   "%NODE%" -v & goto :eof
if "%~1"=="host"   set DSH_HOME=%HOME%& set CICADA_HOME=%HOME%& "%NODE%" "%HOST%" --profile cicada --no-open --port 3123 --host 127.0.0.1
if "%~1"=="probe"  powershell -NoProfile -Command "try{(Invoke-WebRequest -UseBasicParsing http://127.0.0.1:3123/_cicada/ping -TimeoutSec 3).Content}catch{'NO-LISTENER'}"
