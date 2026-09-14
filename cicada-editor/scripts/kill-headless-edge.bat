@echo off
for /f "tokens=2" %%p in ('wmic process where "name='msedge.exe'" get ProcessId^,CommandLine /format:list 2^>nul ^| findstr /i "headless" ^| findstr /i "ProcessId"') do echo %%p
for /f "tokens=2" %%p in ('wmic process where "name='msedge.exe'" get ProcessId^,CommandLine /format:list 2^>nul ^| findstr /i "CommandLine=.*--headless" ^| findstr /i "ProcessId"') do echo %%p
