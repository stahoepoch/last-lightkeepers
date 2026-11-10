[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$Script,
    [string]$EngineRoot = "C:\Program Files\Epic Games\UE_5.8",
    [string]$Project = "",
    [string]$LogPath = ""
)

# Run a Python script against this project with no editor open, through
# UnrealEditor-Cmd's pythonscript commandlet. This is how Content/ gets
# edited when the MCP bridge is down: it is the same editor and the same
# Python API, in a process that exits.
#
# Requires PythonScriptPlugin, which LastLightkeepers.uproject enables. It
# ships prebuilt with the installed engine, so nothing has to be compiled
# for it.
#
# **Two things about output, both found the hard way on 2026-11-09.**
#
# 1. `unreal.log` does NOT reach stdout. It goes to
#    `Saved/Logs/LastLightkeepers.log`. A run piped to a file printed
#    nothing about what it had done while the work was happening fine. The
#    scripts here print to stdout as well for that reason; read both.
# 2. The exit code is better than trap 1 would predict but still not the
#    answer. A Python exception did surface as 255 rather than 0 - so a
#    non-zero code is real - but a script that loads the wrong map, refuses
#    its own safety check, or applies edits and fails the save can exit 0.
#    Read the line the script prints about whether it SAVED.

$ErrorActionPreference = "Stop"
if ([string]::IsNullOrWhiteSpace($Project)) {
    $Project = Join-Path $PSScriptRoot "..\LastLightkeepers.uproject"
}
$editor = Join-Path $EngineRoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
if (-not (Test-Path -LiteralPath $editor)) { throw "UnrealEditor-Cmd.exe not found at '$editor'." }
if (-not (Test-Path -LiteralPath $Script)) { throw "Python script not found at '$Script'." }
$projectPath = (Resolve-Path $Project).Path
$scriptPath = (Resolve-Path $Script).Path.Replace("\", "/")

# An editor holding this project owns the packages we are about to save, and
# a commandlet saving underneath it loses whichever write lands second.
# Failing here is intentional, the same way build_editor.ps1 refuses a Live
# Coding session rather than killing it.
$running = Get-Process -Name "UnrealEditor" -ErrorAction SilentlyContinue
if ($running) {
    throw "An Unreal editor is running (PID $($running[0].Id)). Close it first: a commandlet and an open editor would both write these packages."
}

$editorArgs = @(
    $projectPath,
    "-run=pythonscript",
    "-script=$scriptPath",
    "-stdout",
    "-unattended",
    "-nosplash",
    "-nopause"
)

if ([string]::IsNullOrWhiteSpace($LogPath)) {
    & $editor @editorArgs
} else {
    & $editor @editorArgs | Tee-Object -FilePath $LogPath
}
exit $LASTEXITCODE
