$p = Get-CimInstance Win32_Process -Filter "Name='msedge.exe'" | Where-Object { $_.CommandLine -match 'headless' }
foreach ($x in $p) { Write-Output ("kill {0} {1}" -f $x.ProcessId, ($x.CommandLine.Substring(0, [Math]::Min(80, $x.CommandLine.Length)))) ; Stop-Process -Id $x.ProcessId -Force -ErrorAction SilentlyContinue }
Write-Output ("done, remaining headless: " + ($p | Measure-Object).Count)
