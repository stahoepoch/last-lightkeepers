[CmdletBinding()]
param(
    [string]$EngineRoot = "C:\Program Files\Epic Games\UE_5.8",
    [string]$Project = "",
    [ValidateSet("DebugGame", "Development", "Shipping")]
    [string]$Configuration = "Development",
    [string]$ArchiveDirectory = ""
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($Project)) {
    $Project = Join-Path $PSScriptRoot "..\LastLightkeepers.uproject"
}

$projectPath = (Resolve-Path -LiteralPath $Project).Path
$projectRoot = Split-Path -Parent $projectPath

if ([string]::IsNullOrWhiteSpace($ArchiveDirectory)) {
    # Packages are evidence, not source. Keeping them under Saved prevents a
    # local smoke build from ever becoming an accidental Git/LFS commit.
    $ArchiveDirectory = Join-Path $projectRoot "Saved\Packages\Windows-$Configuration"
}

$uat = Join-Path $EngineRoot "Engine\Build\BatchFiles\RunUAT.bat"
if (-not (Test-Path -LiteralPath $uat)) {
    throw "Unreal Automation Tool was not found at '$uat'. Pass -EngineRoot for the pinned UE install."
}

$runningEditor = Get-Process UnrealEditor, UnrealEditor-Cmd -ErrorAction SilentlyContinue
if ($runningEditor) {
    $processes = ($runningEditor | ForEach-Object { "$($_.ProcessName) ($($_.Id))" }) -join ", "
    throw "Close Unreal before packaging. Running editor process(es): $processes. Packaging against an open editor can cook stale modules."
}

New-Item -ItemType Directory -Force -Path $ArchiveDirectory | Out-Null

# This is intentionally local-only: no Steam credentials, depot upload, or
# publishing flag belongs in a smoke-package tool. Cook, stage and archive are
# enough to prove the project leaves the editor in a runnable Windows layout.
& $uat BuildCookRun `
    "-project=$projectPath" `
    -noP4 `
    -platform=Win64 `
    "-clientconfig=$Configuration" `
    -build -cook -stage -pak -archive `
    "-archivedirectory=$ArchiveDirectory" `
    -utf8output

if ($LASTEXITCODE -ne 0) {
    throw "Windows $Configuration package failed with exit code $LASTEXITCODE. Read the Unreal Automation Tool output above."
}

$exe = Get-ChildItem -LiteralPath $ArchiveDirectory -Filter "LastLightkeepers.exe" -Recurse -File -ErrorAction SilentlyContinue |
    Select-Object -First 1
if ($null -eq $exe) {
    throw "Packaging exited successfully but no LastLightkeepers.exe was found under '$ArchiveDirectory'."
}

Write-Host "Windows package ready: $($exe.FullName)"
exit 0
