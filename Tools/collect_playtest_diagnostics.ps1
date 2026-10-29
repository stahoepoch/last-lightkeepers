[CmdletBinding()]
param(
    # Kept under Saved by default, so diagnostics never dirty the checkout.
    [string]$OutputRoot = (Join-Path $PSScriptRoot "..\Saved\Diagnostics"),
    [switch]$IncludeCrashDumps
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$timestamp = Get-Date -Format 'yyyy-MM-dd_HH-mm-ss'
$bundle = Join-Path $OutputRoot "playtest_$timestamp"
New-Item -ItemType Directory -Force -Path $bundle | Out-Null

function Copy-IfPresent {
    param([string]$Source, [string]$Destination)
    if (Test-Path -LiteralPath $Source) {
        New-Item -ItemType Directory -Force -Path (Split-Path $Destination) | Out-Null
        Copy-Item -LiteralPath $Source -Destination $Destination -Force
        return $true
    }
    return $false
}

function Write-CommandOutput {
    param([string]$Name, [scriptblock]$Command)
    try {
        & $Command 2>&1 | Out-File -FilePath (Join-Path $bundle $Name) -Encoding utf8
    }
    catch {
        "Unable to collect $Name`n$($_.Exception.Message)" |
            Out-File -FilePath (Join-Path $bundle $Name) -Encoding utf8
    }
}

# A report needs to say exactly which code was running and whether local work
# was present. This is intentionally a snapshot, not an attempt to clean or
# modify the checkout.
Write-CommandOutput 'git-revision.txt' { git -C $root rev-parse HEAD }
Write-CommandOutput 'git-status.txt' { git -C $root status --short }
Write-CommandOutput 'system.txt' {
    Get-CimInstance Win32_OperatingSystem | Select-Object Caption, Version, BuildNumber, LastBootUpTime
    Get-CimInstance Win32_ComputerSystem | Select-Object Manufacturer, Model, TotalPhysicalMemory
}

$saved = Join-Path $root 'Saved'
$logsCopied = 0
foreach ($name in @('LastLightkeepers.log', 'LLK-Automation.log')) {
    if (Copy-IfPresent (Join-Path $saved "Logs\$name") (Join-Path $bundle "Logs\$name")) {
        ++$logsCopied
    }
}

$report = Join-Path $saved 'Automation\Report\index.json'
Copy-IfPresent $report (Join-Path $bundle 'Automation\index.json') | Out-Null

if ($IncludeCrashDumps -and (Test-Path -LiteralPath (Join-Path $saved 'Crashes'))) {
    Copy-Item -LiteralPath (Join-Path $saved 'Crashes') -Destination (Join-Path $bundle 'Crashes') -Recurse -Force
}

@(
    "Last Lightkeepers playtest diagnostics",
    "Created: $(Get-Date -Format o)",
    "Logs copied: $logsCopied",
    "Crash dumps included: $IncludeCrashDumps",
    "",
    "Share this folder with the exact symptom and the time it occurred:",
    $bundle
) | Set-Content -Path (Join-Path $bundle 'README.txt') -Encoding utf8

Write-Host "Diagnostics bundle created: $bundle"
