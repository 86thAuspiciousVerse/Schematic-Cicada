@echo off
taskkill /F /IM cicada-engine.exe >nul 2>&1
if not "36508"=="" taskkill /F /PID 36508 >nul 2>&1
