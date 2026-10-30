[CmdletBinding()]
param(
    # By default the script reports whatever evidence is available.  Use this
    # after a completed host-and-client run to make a missing connection a
    # non-zero exit code instead of a warning that can be missed in a console.
    [switch]$RequireConnected,

    # Allows diagnosis of a copied Saved/ folder without opening the project.
    [string]$LogDirectory = '',

    # A loopback run has both logs on one PC and can use Both. On an actual
    # two-machine LAN run, run Host on the host and Client on the joining PC:
    # their Saved folders are separate and pretending otherwise proves nothing.
    [ValidateSet('Auto', 'Host', 'Client', 'Both')]
    [string]$Role = 'Auto'
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
if ([string]::IsNullOrWhiteSpace($LogDirectory)) {
    $LogDirectory = Join-Path $root 'Saved\Logs\LAN'
}

if (-not (Test-Path -LiteralPath $LogDirectory -PathType Container)) {
    throw "No LAN log directory exists at '$LogDirectory'. Run Tools\\launch_lan_smoke.ps1 first."
}

function Get-NewestLog {
    param(
        [Parameter(Mandatory)][string]$Prefix
    )

    return Get-ChildItem -LiteralPath $LogDirectory -File -Filter "$Prefix-*.log" |
        Sort-Object LastWriteTimeUtc -Descending |
        Select-Object -First 1
}

function Get-Evidence {
    param(
        [Parameter(Mandatory)][System.IO.FileInfo]$Log,
        [Parameter(Mandatory)][string[]]$Patterns
    )

    $lines = Get-Content -LiteralPath $Log.FullName
    $hits = @()
    foreach ($pattern in $Patterns) {
        $match = $lines | Select-String -Pattern $pattern -CaseSensitive:$false | Select-Object -Last 1
        if ($null -ne $match) {
            $hits += $match.Line.Trim()
        }
    }
    return @($hits | Select-Object -Unique)
}

$hostLog = Get-NewestLog -Prefix 'Host'
$clientLog = Get-NewestLog -Prefix 'Client'

if ($Role -eq 'Auto') {
    if ($null -ne $hostLog -and $null -ne $clientLog) {
        $Role = 'Both'
    } elseif ($null -ne $hostLog) {
        $Role = 'Host'
    } elseif ($null -ne $clientLog) {
        $Role = 'Client'
    } else {
        throw "No LAN host or client logs found in '$LogDirectory'. Run Tools\\launch_lan_smoke.ps1 first."
    }
}

if (($Role -eq 'Host' -or $Role -eq 'Both') -and $null -eq $hostLog) {
    throw "No host log found in '$LogDirectory'."
}
if (($Role -eq 'Client' -or $Role -eq 'Both') -and $null -eq $clientLog) {
    throw "No client log found in '$LogDirectory'."
}

# UE changes the exact wording between engine versions and network paths, so
# this intentionally accepts several stable lifecycle phrases. It reports the
# actual matching line rather than pretending a particular transport was used.
$hostEvidence = @()
if ($null -ne $hostLog) {
    $hostEvidence = Get-Evidence -Log $hostLog -Patterns @(
        'GameNetDriver.*Listening',
        'NetDriverListen',
        'Accepted connection',
        'Join request'
    )
}
$clientEvidence = @()
if ($null -ne $clientLog) {
    $clientEvidence = Get-Evidence -Log $clientLog -Patterns @(
        'Browse:.*//',
        'PendingNetGame',
        'Join succeeded',
        'Welcomed by server',
        'Join request'
    )
}

Write-Host "LLK LAN smoke inspection ($Role)"
if ($Role -eq 'Host' -or $Role -eq 'Both') {
    Write-Host "Host log:   $($hostLog.FullName)"
    Write-Host "`nHost evidence:"
    if ($hostEvidence.Count -eq 0) { Write-Host '  none found' } else { $hostEvidence | ForEach-Object { Write-Host "  $_" } }
}
if ($Role -eq 'Client' -or $Role -eq 'Both') {
    Write-Host "Client log: $($clientLog.FullName)"
    Write-Host "`nClient evidence:"
    if ($clientEvidence.Count -eq 0) { Write-Host '  none found' } else { $clientEvidence | ForEach-Object { Write-Host "  $_" } }
}

$hostListening = @($hostEvidence | Where-Object { $_ -match 'Listening|NetDriverListen' }).Count -gt 0
$hostAccepted = @($hostEvidence | Where-Object { $_ -match 'Accepted connection|Join request' }).Count -gt 0
$clientJoined = @($clientEvidence | Where-Object { $_ -match 'Join succeeded|Welcomed by server' }).Count -gt 0

$connectionObserved = switch ($Role) {
    'Host' { $hostListening -and $hostAccepted }
    'Client' { $clientJoined }
    'Both' { $hostListening -and $hostAccepted -and $clientJoined }
}

if (($Role -eq 'Host' -or $Role -eq 'Both') -and -not $hostListening) {
    Write-Warning 'The host log does not show a listening server. Check the host window was launched first with -Mode Host.'
}

if ($connectionObserved) {
    if ($Role -eq 'Both') {
        Write-Host "`nPASS: host and client logs contain the expected connection lifecycle evidence."
    } else {
        Write-Host "`nPASS: $Role log contains the expected connection lifecycle evidence."
    }
    exit 0
}

$message = if ($Role -eq 'Both') {
    'No loopback connection is proven by these logs yet. Inspect both game windows, then run again after they have been closed.'
} else {
    "No connection is proven by this $Role log yet. Inspect this game window and its firewall/LAN settings, then run again after it has been closed."
}
if ($RequireConnected) {
    throw $message
}
Write-Warning $message
