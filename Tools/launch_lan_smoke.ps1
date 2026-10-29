[CmdletBinding()]
param(
    [Parameter(Mandatory)]
    [ValidateSet('Host', 'Client')]
    [string]$Mode,

    # The host's private-LAN IPv4 address, for example 192.168.1.42. Required
    # only by Client; this deliberately does not involve Steam or an App ID.
    [string]$HostAddress = '',

    [string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8',
    [switch]$DryRun
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$project = Join-Path $root 'LastLightkeepers.uproject'
$editor = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor.exe'

if (-not (Test-Path -LiteralPath $project)) {
    throw "Missing project file: $project"
}
if (-not (Test-Path -LiteralPath $editor)) {
    throw "Pinned Unreal Editor was not found: $editor"
}
if (Get-Process UnrealEditor -ErrorAction SilentlyContinue) {
    throw 'Unreal Editor is already open. Close it before a standalone LAN run so Live Coding cannot leave either machine on stale modules.'
}
if ($Mode -eq 'Client' -and [string]::IsNullOrWhiteSpace($HostAddress)) {
    throw 'Client mode needs -HostAddress, for example: -HostAddress 192.168.1.42'
}
if ($Mode -eq 'Client' -and $HostAddress -notmatch '^\d{1,3}(?:\.\d{1,3}){3}(?::\d+)?$') {
    throw "HostAddress must be a LAN IPv4 address with an optional port, not '$HostAddress'."
}

$logDir = Join-Path $root 'Saved\Logs\LAN'
New-Item -ItemType Directory -Force -Path $logDir | Out-Null
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$log = Join-Path $logDir "$Mode-$stamp.log"

function Get-PrivateLanAddresses {
    # Do not guess which adapter is "the" network. VPNs, Docker and virtual
    # switches are common on a development PC, so show every plausible private
    # address and let the other machine use the one reachable on its network.
    $private = '^(10\.|192\.168\.|172\.(1[6-9]|2[0-9]|3[0-1])\.)'
    return @(
        Get-NetIPAddress -AddressFamily IPv4 -ErrorAction SilentlyContinue |
            Where-Object {
                $_.IPAddress -match $private -and
                $_.InterfaceAlias -notmatch 'Loopback'
            } |
            Sort-Object InterfaceMetric, InterfaceAlias |
            Select-Object -ExpandProperty IPAddress -Unique
    )
}

# The harness GameMode builds its floor on any map, so the engine's Entry map is
# sufficient for this networking proof. A real M1.5 store map replaces only
# this map argument later; the host/client topology and the evidence stay same.
$target = if ($Mode -eq 'Host') { '/Engine/Maps/Entry?listen' } else { $HostAddress }
$arguments = @(
    "`"$project`"",
    $target,
    '-game',
    '-windowed',
    '-ResX=1280',
    '-ResY=720',
    '-log',
    "-abslog=`"$log`""
)

Write-Host "LLK LAN smoke test: $Mode"
Write-Host "Log: $log"
Write-Host "Command: `"$editor`" $($arguments -join ' ')"

if ($Mode -eq 'Host') {
    $addresses = Get-PrivateLanAddresses
    Write-Host "`nGive the second computer one of these client commands:"
    if ($addresses.Count -eq 0) {
        Write-Warning 'No private LAN IPv4 address was found. Connect this PC to your local network, then run ipconfig if you need to identify it manually.'
    } else {
        $addresses | ForEach-Object {
            Write-Host "  .\Tools\launch_lan_smoke.ps1 -Mode Client -HostAddress $_"
        }
    }
}

if ($DryRun) {
    Write-Host 'Dry run only: no game window launched.'
    exit 0
}

# Visible is intentional: this is a human playtest, not background tooling.
Start-Process -FilePath $editor -ArgumentList $arguments -WorkingDirectory $root
Write-Host "Started $Mode. When the test is over, run Tools\inspect_lan_smoke.ps1 -RequireConnected on the host, then Tools\collect_playtest_diagnostics.ps1 on each machine."
