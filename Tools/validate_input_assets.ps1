[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$project = Join-Path $root 'LastLightkeepers.uproject'
$editor = 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$log = Join-Path $root 'Saved\Logs\LLK-InputValidation.log'

if (-not (Test-Path -LiteralPath $editor)) {
    throw "Pinned Unreal commandlet executable was not found: $editor"
}

& $editor $project '-run=LLKValidateInput' '-unattended' '-nop4' '-nosplash' "-abslog=$log"
if ($LASTEXITCODE -ne 0) {
    throw "LLK input asset validation failed. Read $log"
}

Write-Host "LLK input asset contract passed. Log retained at $log"
