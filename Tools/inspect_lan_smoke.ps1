[CmdletBinding()]
param(
    # By default the script reports whatever evidence is available.  Use this
    # after a completed host-and-client run to make a missing connection a
    # non-zero exit code instead of a warning that can be missed in a console.
    [switch]$RequireConnected,

    # Allows diagnosis of a copied Saved/ folder without opening the project.
    [string]$LogDirectory = ''
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

if ($null -eq $hostLog) {
    throw "No host log found in '$LogDirectory'."
}
if ($null -eq $clientLog) {
    throw "No client log found in '$LogDirectory'."
}

# UE changes the exact wording between engine versions and network paths, so
# this intentionally accepts several stable lifecycle phrases. It reports the
# actual matching line rather than pretending a particular transport was used.
$hostEvidence = Get-Evidence -Log $hostLog -Patterns @(
    'GameNetDriver.*Listening',
    'NetDriverListen',
    'Accepted connection',
    'Join request'
)
$clientEvidence = Get-Evidence -Log $clientLog -Patterns @(
    'Browse:.*//',
    'PendingNetGame',
    'Join succeeded',
    'Welcomed by server',
    'Join request'
)

Write-Host 'LLK LAN smoke inspection'
Write-Host "Host log:   $($hostLog.FullName)"
Write-Host "Client log: $($clientLog.FullName)"

Write-Host "`nHost evidence:"
if ($hostEvidence.Count -eq 0) { Write-Host '  none found' } else { $hostEvidence | ForEach-Object { Write-Host "  $_" } }

Write-Host "`nClient evidence:"
if ($clientEvidence.Count -eq 0) { Write-Host '  none found' } else { $clientEvidence | ForEach-Object { Write-Host "  $_" } }

$hostListening = $hostEvidence | Where-Object { $_ -match 'Listening|NetDriverListen' }
$connectionObserved = ($hostEvidence | Where-Object { $_ -match 'Accepted connection|Join request' }).Count -gt 0 -and $clientEvidence.Count -gt 0

if ($hostListening.Count -eq 0) {
    Write-Warning 'The host log does not show a listening server. Check the host window was launched first with -Mode Host.'
}

if ($connectionObserved) {
    Write-Host "`nPASS: host and client logs both contain connection lifecycle evidence."
    exit 0
}

$message = 'No cross-machine connection is proven by these logs yet. This does not prove the test failed; inspect both game windows and firewall/LAN settings, then run again after both have been closed.'
if ($RequireConnected) {
    throw $message
}
Write-Warning $message
