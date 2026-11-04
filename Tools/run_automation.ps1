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

function Restore-ConfigAfterGeneratedAndroidFileServerToken {
    param(
        [Parameter(Mandatory)][string]$EngineConfig,
        [Parameter(Mandatory)][byte[]]$BeforeAutomation
    )

    # UnrealEditor-Cmd itself rewrites DefaultEngine.ini before this cleanup
    # runs: it removes the UTF-8 BOM and leaves a blank line when its Android
    # File Server token is removed. Snapshotting the exact bytes before launch
    # is the only way to leave a successful automation run truly clean.
    $after = [System.IO.File]::ReadAllText($EngineConfig)
    if ($after -match '(?m)^SecurityToken=') {
        [System.IO.File]::WriteAllBytes($EngineConfig, $BeforeAutomation)
        Write-Host "Restored tracked project config after removing a generated Android File Server token."
    }
}

$engineConfig = Join-Path (Split-Path $projectPath) "Config\DefaultEngine.ini"
$configBeforeAutomation = $null
if (Test-Path -LiteralPath $engineConfig) {
    $configBeforeText = [System.IO.File]::ReadAllText($engineConfig)
    if ($configBeforeText -match '(?m)^SecurityToken=') {
        throw 'DefaultEngine.ini already contains a machine-generated Android File Server token. Remove it before running automation.'
    }
    $configBeforeAutomation = [System.IO.File]::ReadAllBytes($engineConfig)
}

Remove-Item -LiteralPath $report -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -LiteralPath $log -Force -ErrorAction SilentlyContinue

# -TestExit makes the editor close only when Automation has completed.  Do not
# replace it with a timed wait: a timeout can make an unfinished suite look
# green. The report is parsed below because editor process exit codes alone do
# not reliably express individual automation failures on every UE minor.
& $editor "-project=$projectPath" "-ExecCmds=Automation RunTests $TestFilter; Quit" "-TestExit=Automation Test Queue Empty" "-ReportExportPath=$report" "-abslog=$log" -unattended -nopause -nosplash -nullrhi -nosound
$editorExit = $LASTEXITCODE
if ($null -ne $configBeforeAutomation) {
    Restore-ConfigAfterGeneratedAndroidFileServerToken -EngineConfig $engineConfig -BeforeAutomation $configBeforeAutomation
}

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

# A clean editor exit is not a passing suite: this toolchain has returned 0
# for a reported failure. The checks above reject reported failures and an
# empty selection. This is the positive proof: AutomationController records
# one completed, successful test body. Do not require a project-specific log
# marker here; ordinary IMPLEMENT_SIMPLE_AUTOMATION_TEST bodies do not emit
# one, and treating their real Result={Success} as a failure makes the runner
# lie in the opposite direction.
if ($logText -notmatch "LogAutomationController: Display: Test Completed\. Result=\{Success\}") {
    Write-Error "LLK automation did not prove that any requested test body completed successfully. Check '$log'."
    exit 1
}

Write-Host "LLK automation passed: $TestFilter"
if (-not $KeepLogs) { Write-Host "Log retained at $log for inspection." }
exit 0
