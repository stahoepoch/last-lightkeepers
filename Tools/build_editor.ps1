[CmdletBinding()]
param(
    [string]$EngineRoot = "C:\Program Files\Epic Games\UE_5.8",
    [string]$Project = ""
)

$ErrorActionPreference = "Stop"
if ([string]::IsNullOrWhiteSpace($Project)) {
    $Project = Join-Path $PSScriptRoot "..\LastLightkeepers.uproject"
}
$build = Join-Path $EngineRoot "Engine\Build\BatchFiles\Build.bat"
if (-not (Test-Path -LiteralPath $build)) { throw "UE build script not found at '$build'." }
$projectPath = (Resolve-Path $Project).Path

# A Live Coding session owns the module DLLs. Failing here is intentional: do
# not kill a teammate's running editor or pretend a stale DLL was rebuilt.
& $build LastLightkeepersEditor Win64 Development "-Project=$projectPath" -NoHotReload
exit $LASTEXITCODE
