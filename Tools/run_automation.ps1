[CmdletBinding()]
param(
    [string]$EngineRoot = "C:\Program Files\Epic Games\UE_5.8",
    [string]$Project = "",
    [string]$TestFilter = "LLK",
    [switch]$KeepLogs
)

$ErrorActionPreference = "Stop"
if ([string]::IsNullOrWhiteSpace($Project)) {
    $Project = Join-Path $PSScriptRoot "..\LastLightkeepers.uproject"
}
$projectPath = (Resolve-Path $Project).Path
$editor = Join-Path $EngineRoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
if (-not (Test-Path -LiteralPath $editor)) {
    throw "UnrealEditor-Cmd.exe was not found at '$editor'. Pass -EngineRoot for the pinned UE 5.8.4 install."
}

$report = Join-Path (Split-Path $projectPath) "Saved\Automation\Report"
$log = Join-Path (Split-Path $projectPath) "Saved\Logs\LLK-Automation.log"
Remove-Item -LiteralPath $report -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -LiteralPath $log -Force -ErrorAction SilentlyContinue

# -TestExit makes the editor close only when Automation has completed.  Do not
# replace it with a timed wait: a timeout can make an unfinished suite look
# green. The report is parsed below because editor process exit codes alone do
# not reliably express individual automation failures on every UE minor.
& $editor "-project=$projectPath" "-ExecCmds=Automation RunTests $TestFilter; Quit" "-TestExit=Automation Test Queue Empty" "-ReportExportPath=$report" "-abslog=$log" -unattended -nopause -nosplash -nullrhi -nosound
$editorExit = $LASTEXITCODE

if (-not (Test-Path -LiteralPath $log)) {
    throw "Automation produced no log at '$log'. The editor exited with $editorExit."
}

$logText = Get-Content -LiteralPath $log -Raw
$failureLines = @($logText -split "`r?`n" | Where-Object {
    $_ -match "LogAutomationController: (Error|Warning:.*(failed|no tests))" -or
    $_ -match "Automation.*(failed|Failed)"
})

# An empty test selection is a broken harness, not a passing suite.
if ($logText -match "No automation tests matched|0 tests.*run|No tests were found") {
    $failureLines += "No automation tests matched '$TestFilter'."
}

if ($editorExit -ne 0) {
    $failureLines += "UnrealEditor-Cmd exited with code $editorExit."
}

if ($failureLines.Count -gt 0) {
    Write-Error ("LLK automation failed:`n" + ($failureLines -join "`n"))
    exit 1
}

if ($logText -notmatch "LLK automation harness reached RunTest") {
    Write-Error "LLK automation did not prove that the test body ran. Check '$log'."
    exit 1
}

Write-Host "LLK automation passed: $TestFilter"
if (-not $KeepLogs) { Write-Host "Log retained at $log for inspection." }
exit 0
