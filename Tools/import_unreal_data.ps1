[CmdletBinding()]
param(
    [string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8'
)

$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$project = Join-Path $root 'LastLightkeepers.uproject'
$payloadDir = Join-Path $root 'Intermediate\LLKDataImport'
$manifestPath = Join-Path $payloadDir 'asset-manifest.json'
$dataDirectory = Join-Path $root 'Content\Data'
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

    if (-not (Test-Path -LiteralPath $manifestPath)) {
        throw "Importer emitted no asset manifest: $manifestPath"
    }
    $manifest = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
    $expectedAssetNames = @(
        @($manifest.DataTables) + @($manifest.DataAssets) |
            ForEach-Object { "$($_.asset).uasset" }
    )
    if ($expectedAssetNames.Count -eq 0) {
        throw "Importer manifest names no generated assets: $manifestPath"
    }

    # Content/Data is exclusively ignored build output. A type or asset rename
    # otherwise leaves the old .uasset beside its replacement, which makes a
    # successful import look non-idempotent and can cook obsolete data. Delete
    # only direct .uasset children not named by the current emitted manifest;
    # never recurse, and never touch authored Content outside this directory.
    if (Test-Path -LiteralPath $dataDirectory) {
        $staleAssets = @(
            Get-ChildItem -LiteralPath $dataDirectory -File -Filter '*.uasset' |
                Where-Object { $_.Name -notin $expectedAssetNames }
        )
        foreach ($asset in $staleAssets) {
            Write-Host "Removing stale generated data asset: $($asset.Name)"
            Remove-Item -LiteralPath $asset.FullName -Force
        }
    }

    & $editor $project '-run=LLKImportData' ("-InputDir=`"$payloadDir`"") `
        '-unattended' '-nop4' '-nosplash' ("-abslog=$log")
    if ($LASTEXITCODE -ne 0) { throw "Unreal data import failed ($LASTEXITCODE). See $log" }

    $outputs = Get-ChildItem -LiteralPath $dataDirectory -Filter '*.uasset' -File
    $missingAssets = @($expectedAssetNames | Where-Object { $_ -notin $outputs.Name })
    if ($outputs.Count -ne $expectedAssetNames.Count -or $missingAssets.Count -gt 0) {
        throw "Expected generated assets [$($expectedAssetNames -join ', ')], found [$($outputs.Name -join ', ')]."
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
