# Render stills of a map from named camera positions, several per invocation.
#
# Chris is remote and cannot play, so a rendered frame is the only way either
# of us sees this game. `docs/FINDINGS-THE-STORE-IS-UNLIT-NOT-EMPTY.md` found
# the one-command recipe; this is that recipe made repeatable, after two
# things about it cost most of an afternoon on 2026-11-22.
#
# **Every ExecCmds entry fires on the same frame.** A list of
# "BugItGo A, HighResShot, BugItGo B, HighResShot" does not give two shots
# from two places - it gives one, because nothing between them waits. So one
# launch is one shot, and several shots are several launches.
#
# **`-game` never exits.** Without a quit the process sits at the frame after
# the capture for as long as you let it, and a loop of them looks like a slow
# render rather than a queue of live games. This watches for the PNG and stops
# the process itself, which turns ~8 minutes a shot into about one.
#
#   .\Tools\shoot_map.ps1 -Map /Game/LLK_Boswells_OpeningPass -Shots @{
#       aisle = "0 -1200 160 20 90 0"; wing = "-2600 -800 160 25 0 0" }
#
# Each value is "X Y Z Pitch Yaw Roll", exactly as BugItGo takes it. PNGs land
# beside the project in Saved/Screenshots/WindowsEditor as <name>.png.

param(
    [Parameter(Mandatory = $true)][string]$Map,
    [Parameter(Mandatory = $true)][hashtable]$Shots,
    [string]$Project = "C:\Users\chris\Documents\Last-Lightkeepers\LastLightkeepers.uproject",
    [string]$Editor = "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe",
    [int]$TimeoutSeconds = 420,
    [string]$Extra = "",
    # Console commands run after the camera is placed and before the capture,
    # e.g. "LLK.Power 1" to photograph the store with the mains on. They fire
    # on the same frame as the shot, which is what you want for a state the
    # game applies immediately and not for one that animates.
    [string]$Then = ""
)

$ErrorActionPreference = "Stop"
if (-not (Test-Path $Editor)) { throw "shoot_map: no editor at $Editor" }
if (-not (Test-Path $Project)) { throw "shoot_map: no project at $Project" }

$dir = Join-Path (Split-Path $Project) "Saved\Screenshots\WindowsEditor"
New-Item -ItemType Directory -Force -Path $dir | Out-Null

foreach ($name in $Shots.Keys) {
    Get-ChildItem "$dir\HighresScreenshot*.png" -ErrorAction SilentlyContinue |
        Remove-Item -Force -ErrorAction SilentlyContinue

    $cmds = "ghost, BugItGo $($Shots[$name])"
    if ($Then) { $cmds += ", $Then" }
    $cmds += ", HighResShot 1600x900"
    # The quotes around $cmds are load-bearing. Start-Process splits each
    # ArgumentList element on spaces, so an unquoted "-ExecCmds=ghost, BugItGo
    # -2600 -800 ..." arrives at the engine as eight separate arguments, the
    # camera never moves and the capture never fires. Two 180 s timeouts.
    $args = @("`"$Project`"", $Map, "-game", "-windowed", "-resx=1600",
              "-resy=900", "-ExecCmds=`"$cmds`"", "-nosplash", "-nopause")
    if ($Extra) { $args += $Extra.Split(" ") }

    # Normal, not Minimized. A minimized window stops presenting frames and
    # the capture never happens - the first run of this waited out a 420 s
    # timeout on a shot that was never going to render.
    $p = Start-Process -FilePath $Editor -ArgumentList $args -PassThru

    # The capture is the signal, not the exit code: a -game process that has
    # written the PNG has done the whole job and is only still drawing frames.
    $shot = $null
    $deadline = (Get-Date).AddSeconds($TimeoutSeconds)
    while ((Get-Date) -lt $deadline) {
        Start-Sleep -Milliseconds 800
        $shot = Get-ChildItem "$dir\HighresScreenshot*.png" -ErrorAction SilentlyContinue |
                Select-Object -First 1
        if ($shot) {
            # The file appears before it is finished being written. Wait for
            # its length to settle rather than racing the encoder.
            $size = -1
            while ($shot.Length -ne $size) {
                $size = $shot.Length
                Start-Sleep -Milliseconds 400
                $shot.Refresh()
            }
            break
        }
        if ($p.HasExited) { break }
    }

    if (-not $p.HasExited) { Stop-Process -Id $p.Id -Force -ErrorAction SilentlyContinue }

    if ($shot) {
        Move-Item $shot.FullName (Join-Path $dir "$name.png") -Force
        "shoot_map $name OK $($shot.Length) bytes"
    } else {
        "shoot_map $name FAILED (no capture within $TimeoutSeconds s)"
    }
}
