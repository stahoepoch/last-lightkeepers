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

    # **Relaxed locally, and this cost a collected diff to find.** In Windows
    # PowerShell 5.1 a native command's stderr line, once redirected with
    # `2>&1`, arrives as an ErrorRecord - and under the script's
    # $ErrorActionPreference = 'Stop' that is terminating. `git rev-parse`
    # writes nothing to stderr and so looked fine; `git diff` emits a harmless
    # "CRLF will be replaced by LF" warning, which discarded the entire diff
    # and wrote "Unable to collect" in its place. The preference is restored
    # afterwards so the rest of the script keeps failing loudly.
    $previous = $ErrorActionPreference
    $ErrorActionPreference = 'Continue'
    try {
        & $Command 2>&1 | Out-File -FilePath (Join-Path $bundle $Name) -Encoding utf8
    }
    catch {
        "Unable to collect $Name`n$($_.Exception.Message)" |
            Out-File -FilePath (Join-Path $bundle $Name) -Encoding utf8
    }
    finally {
        $ErrorActionPreference = $previous
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

# Whether the BINARY matches the source, which git cannot answer and which is
# the commonest explanation for "it looks wrong": you are running a stale
# build. Compared here rather than left for someone to work out from two
# timestamps in two different files.
Write-CommandOutput 'build.txt' {
    # The PROJECT's Binaries, not the engine install. A project module builds
    # here; only the editor executable itself lives under the engine, and
    # taking the engine path reported "not found" against a DLL that was
    # sitting in the project the whole time.
    $dll = Join-Path $root 'Binaries\Win64\UnrealEditor-LLK.dll'
    if (-not (Test-Path -LiteralPath $dll)) {
        "UnrealEditor-LLK.dll not found at $dll - the module has never been built here."
    }
    else {
        $built = (Get-Item -LiteralPath $dll).LastWriteTime
        "UnrealEditor-LLK.dll built: $($built.ToString('o'))"

        # **Compared against the SOURCE, not against HEAD.** The first version
        # of this compared the binary to the latest commit time, which cries
        # wolf on every healthy session: the normal order is edit, build,
        # test, commit, so the DLL legitimately predates the commit by a
        # minute or two every single time. A warning that fires when nothing
        # is wrong is a warning somebody learns to scroll past.
        #
        # Whether a source file is newer than the binary is the actual
        # question, and it does not care when anybody committed.
        # Headers, sources, and the build config - .Build.cs and .Target.cs all
        # live under Source/, so the *.cs include covers "a dependency changed"
        # as well as "code changed". Both require a rebuild and neither shows
        # up as a changed .cpp.
        $sources = @(Get-ChildItem -LiteralPath (Join-Path $root 'Source') -Recurse -File -Include '*.h', '*.cpp', '*.cs' -ErrorAction SilentlyContinue)

        # Plus the .uproject, on Codex's framing of this check as "the latest
        # runtime or build-config change". Enabling or disabling a plugin
        # changes what is compiled and linked while touching nothing under
        # Source/ - and this project has already done exactly that once,
        # disabling the Fab plugin during M0.7. A rebuild is needed and no
        # source file would say so.
        $sources += @(Get-ChildItem -LiteralPath $root -File -Filter '*.uproject' -ErrorAction SilentlyContinue)

        $newest = $sources | Sort-Object LastWriteTime -Descending | Select-Object -First 1
        if ($newest) {
            "Newest build input:         $($newest.LastWriteTime.ToString('o'))  ($($newest.Name))"
            if ($newest.LastWriteTime -gt $built) {
                "STALE BUILD: $($newest.Name) is newer than the binary."
                "The editor was built before that change, so it is not running what is on"
                "disk. Rebuild before trusting anything else in this bundle."
            }
            else {
                "Build is current: no build input is newer than the binary."
            }
        }

        # Recorded, not compared. Useful for saying which commit the build
        # corresponds to; useless as a staleness test, per the note above.
        "HEAD committed:             $(git -C $root log -1 --format=%cI)"
    }
}

# What the local changes ACTUALLY are. `git status --short` says that work is
# present; it does not say what, and "there were local changes" is not a
# diagnosis. Read-only, and scoped to the text that decides behaviour so a
# regenerated .uasset cannot bury it.
Write-CommandOutput 'git-diff.txt' {
    # stderr to $null rather than into the file. git emits a "CRLF will be
    # replaced by LF" warning on this repo, and merged into the output stream
    # PowerShell renders it as a multi-line RemoteException block at the top
    # of the diff - the first thing read, and not the thing wanted. The exit
    # code below is what reports a real failure.
    git -C $root diff --stat -- Source Config data Tools tests 2>$null
    "(git diff --stat exit code: $LASTEXITCODE)"
    ""
    "--- full diff ---"
    git -C $root diff -- Source Config data Tools tests 2>$null
}

# **The newest logs by modification time, not by name.** The realistic
# sequence after a freeze is force-quit, reopen, then remember to run this -
# at which point the live LastLightkeepers.log is a fresh session and the
# freeze is in LastLightkeepers-backup-<stamp>.log. Taking the live one by
# name collected the wrong file and said "Logs copied: 2" while doing it.
#
# Also picks up LastLightkeepers_2.log, the SECOND editor or PIE instance -
# which is the co-op case this whole script exists for, and which a
# by-name list of the first instance misses entirely. LAN smoke runs write
# their host/client logs one level down under Saved\Logs\LAN; preserve that
# directory in the bundle so identically named logs can never overwrite.
$logsCopied = 0
$logDir = Join-Path $saved 'Logs'
if (Test-Path -LiteralPath $logDir) {
    # Ours only. Saved\Logs also holds cef3.log and similar, which are not
    # about this game and would take slots from the sessions that are.
    $recent = Get-ChildItem -LiteralPath $logDir -Recurse -Filter '*.log' |
        Where-Object {
            $_.Name -like 'LastLightkeepers*' -or $_.Name -like 'LLK-*' -or
            $_.DirectoryName -eq (Join-Path $logDir 'LAN')
        } |
        Sort-Object LastWriteTime -Descending |
        Select-Object -First 8
    foreach ($log in $recent) {
        $relativeLogPath = $log.FullName.Substring($logDir.Length).TrimStart('\')
        if (Copy-IfPresent $log.FullName (Join-Path $bundle "Logs\$relativeLogPath")) {
            ++$logsCopied
        }
    }
}

$report = Join-Path $saved 'Automation\Report\index.json'
Copy-IfPresent $report (Join-Path $bundle 'Automation\index.json') | Out-Null

# A FAILING automation run archives its log, report and failure summary under
# Automation\History (the evidence-retention work). That is the half worth
# having when tests are red, and Report\index.json alone is the half that is
# overwritten by the next run.
$history = Join-Path $saved 'Automation\History'
$historyCopied = 0
if (Test-Path -LiteralPath $history) {
    $runs = Get-ChildItem -LiteralPath $history -Directory |
        Sort-Object LastWriteTime -Descending |
        Select-Object -First 2
    foreach ($run in $runs) {
        Copy-Item -LiteralPath $run.FullName -Destination (Join-Path $bundle "Automation\History\$($run.Name)") -Recurse -Force
        ++$historyCopied
    }
}

if ($IncludeCrashDumps -and (Test-Path -LiteralPath (Join-Path $saved 'Crashes'))) {
    Copy-Item -LiteralPath (Join-Path $saved 'Crashes') -Destination (Join-Path $bundle 'Crashes') -Recurse -Force
}

# Resolved, because this is the path that gets pasted into a message and
# "Tools\..\Saved" is a path somebody then has to mentally simplify.
$bundlePath = (Resolve-Path -LiteralPath $bundle).Path

@(
    "Last Lightkeepers playtest diagnostics",
    "Created: $(Get-Date -Format o)",
    "Logs copied: $logsCopied (newest first, by modification time - includes",
    "  LastLightkeepers_2.log for a second instance, LAN host/client logs, and rotated -backup- logs,",
    "  because after a force-quit the freeze is in the backup and not the live one)",
    "Archived automation runs copied: $historyCopied",
    "Crash dumps included: $IncludeCrashDumps",
    "",
    "Read build.txt FIRST. If it says STALE BUILD, the symptom may already be",
    "explained and nothing else in here is trustworthy.",
    "",
    "Share this folder with the exact symptom and the time it occurred:",
    $bundlePath
) | Set-Content -Path (Join-Path $bundle 'README.txt') -Encoding utf8

Write-Host "Diagnostics bundle created: $bundlePath"
