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
$historyRoot = Join-Path (Split-Path $projectPath) "Saved\Automation\History"

function Save-FailureEvidence {
    param([string[]]$Reasons)

    $stamp = Get-Date -Format "yyyyMMdd-HHmmss"
    $destination = Join-Path $historyRoot "failed-$stamp"
    New-Item -ItemType Directory -Force -Path $destination | Out-Null
    if (Test-Path -LiteralPath $log) { Copy-Item -LiteralPath $log -Destination (Join-Path $destination "LLK-Automation.log") }
    if (Test-Path -LiteralPath $report) { Copy-Item -LiteralPath $report -Destination (Join-Path $destination "Report") -Recurse }
    $Reasons | Set-Content -LiteralPath (Join-Path $destination "failure.txt") -Encoding utf8
    return $destination
}

function Clear-GeneratedAndroidFileServerToken {
    $engineConfig = Join-Path (Split-Path $projectPath) "Config\DefaultEngine.ini"
    if (-not (Test-Path -LiteralPath $engineConfig)) { return }

    # This file is deliberately source-controlled.  Removing an editor-made
    # Android File Server token must not also rewrite its line endings, remove
    # its final newline, or flip its UTF-8 BOM; any of those makes an
    # automation run look like a project-config change.
    $rawBytes = [System.IO.File]::ReadAllBytes($engineConfig)
    $hasUtf8Bom = $rawBytes.Length -ge 3 -and
        $rawBytes[0] -eq 0xEF -and $rawBytes[1] -eq 0xBB -and $rawBytes[2] -eq 0xBF
    $contents = [System.IO.File]::ReadAllText($engineConfig)
    $clean = $contents -replace '(?m)^SecurityToken=.*\r?\n?', ''
    if ($clean -ne $contents) {
        $encoding = New-Object System.Text.UTF8Encoding($hasUtf8Bom)
        [System.IO.File]::WriteAllText($engineConfig, $clean, $encoding)
        Write-Host "Removed a generated Android File Server token from tracked project config."
    }
}

Remove-Item -LiteralPath $report -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -LiteralPath $log -Force -ErrorAction SilentlyContinue

# -TestExit makes the editor close only when Automation has completed.  Do not
# replace it with a timed wait: a timeout can make an unfinished suite look
# green. The report is parsed below because editor process exit codes alone do
# not reliably express individual automation failures on every UE minor.
& $editor "-project=$projectPath" "-ExecCmds=Automation RunTests $TestFilter; Quit" "-TestExit=Automation Test Queue Empty" "-ReportExportPath=$report" "-abslog=$log" -unattended -nopause -nosplash -nullrhi -nosound
$editorExit = $LASTEXITCODE
Clear-GeneratedAndroidFileServerToken

if (-not (Test-Path -LiteralPath $log)) {
    $evidence = Save-FailureEvidence @("Automation produced no log at '$log'.", "UnrealEditor-Cmd exited with code $editorExit.")
    throw "Automation produced no log at '$log'. The editor exited with $editorExit. Failure evidence: '$evidence'."
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
    $evidence = Save-FailureEvidence $failureLines
    Write-Error ("LLK automation failed (evidence: $evidence):`n" + ($failureLines -join "`n"))
    exit 1
}

if ($logText -notmatch "LLK automation harness reached RunTest") {
    Write-Error "LLK automation did not prove that the test body ran. Check '$log'."
    exit 1
}

Write-Host "LLK automation passed: $TestFilter"
if (-not $KeepLogs) { Write-Host "Log retained at $log for inspection." }
exit 0
