[CmdletBinding()]
param(
    [string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8'
)

$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$project = Join-Path $root 'LastLightkeepers.uproject'
$payloadDir = Join-Path $root 'Intermediate\LLKDataImport'
$log = Join-Path $root 'Saved\Logs\LLK-DataImport.log'
$editor = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'

if (-not (Test-Path -LiteralPath $project)) {
    throw "Missing Unreal project: $project"
}
if (-not (Test-Path -LiteralPath $editor)) {
    throw "Missing Unreal command-line editor: $editor"
}
if (Get-Process UnrealEditor -ErrorAction SilentlyContinue) {
    throw 'Unreal Editor is open. Close it before importing generated data assets.'
}

Push-Location $root
try {
    & python (Join-Path $PSScriptRoot 'import_data.py') --emit
    if ($LASTEXITCODE -ne 0) { throw "Portable data validation failed ($LASTEXITCODE)." }

    & $editor $project '-run=LLKImportData' ("-InputDir=`"$payloadDir`"") `
        '-unattended' '-nop4' '-nosplash' ("-abslog=$log")
    if ($LASTEXITCODE -ne 0) { throw "Unreal data import failed ($LASTEXITCODE). See $log" }

    $outputs = Get-ChildItem -LiteralPath (Join-Path $root 'Content\Data') -Filter '*.uasset' -File
    if ($outputs.Count -ne 11) {
        throw "Expected 11 generated assets in Content/Data, found $($outputs.Count)."
    }
    foreach ($output in $outputs) {
        git -C $root check-ignore -q -- $output.FullName
        if ($LASTEXITCODE -ne 0) { throw "Generated asset is not ignored by Git: $($output.FullName)" }
    }

    Write-Host 'LLK data import succeeded: 11 generated assets are current and ignored by Git.'
}
finally {
    Pop-Location
}
