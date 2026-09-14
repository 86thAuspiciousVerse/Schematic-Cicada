@echo off
rem Create the desktop shortcut for the product window (docs/04 sec.4).
rem Target = start-cicada-app.bat, icon = assets\brand\cicada.ico.
setlocal
set ROOT=C:\dsh\Schematic-Cicada
powershell.exe -NoProfile -ExecutionPolicy Bypass -Command ^
  "$ws = New-Object -ComObject WScript.Shell; $lnk = $ws.CreateShortcut([Environment]::GetFolderPath('Desktop') + '\Schematic-Cicada.lnk'); $lnk.TargetPath = '%ROOT%\cicada-editor\scripts\start-cicada-app.bat'; $lnk.WorkingDirectory = '%ROOT%'; $lnk.IconLocation = '%ROOT%\assets\brand\cicada.ico'; $lnk.Description = 'Schematic-Cicada'; $lnk.Save(); Write-Output ('shortcut: ' + $lnk.FullName)"
exit /b %ERRORLEVEL%
