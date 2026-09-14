@echo off
rem Close a running Schematic-Cicada shell (Electron) AND its supervised stack.
rem Needed because the shell is single-instance and tray-resident: after an update
rem the OLD instance keeps serving, so a re-launch only focuses stale UI.
rem Matching is precise: Electron processes whose image lives in cicada-shell, plus
rem the node launcher tree (cicada-app.ts) that owns engine/host children.
rem usage: close-shell.bat
setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -Command ^
  "$launchers = Get-CimInstance Win32_Process -Filter \"Name='node.exe'\" | Where-Object { $_.CommandLine -like '*cicada-app.ts*' };" ^
  "foreach ($p in $launchers) { Write-Host ('[close-shell] launcher tree pid ' + $p.ProcessId); taskkill /PID $p.ProcessId /T /F | Out-Null }" ^
  "$shells = Get-Process electron -ErrorAction SilentlyContinue | Where-Object { $_.Path -like '*\cicada-shell\*' };" ^
  "foreach ($s in $shells) { Write-Host ('[close-shell] shell pid ' + $s.Id + ' ' + $s.MainWindowTitle); Stop-Process -Id $s.Id -Force }" ^
  "if (-not $launchers -and -not $shells) { Write-Host '[close-shell] nothing running' }"
echo [close-shell] done
exit /b 0
