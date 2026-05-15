#Requires -Version 5.0
<#
.SYNOPSIS
  Build (optional run) this project without MSBuild on PATH.

.DESCRIPTION
  - Finds latest VS via vswhere
  - MSBuild Release|x64 by default
  - -Run starts exe from repo root so res/ loads

  One-time machine setup:
  - VS workload: Desktop development with C++
  - EasyX: https://easyx.cn/

.PARAMETER Run
  After successful build, launch game from project root.

.PARAMETER Configuration
  Debug or Release (default Release).

.PARAMETER Platform
  x64 or x86 (default x64; x86 maps to Win32 for MSBuild).

.PARAMETER ProjectRoot
  Optional absolute path to this repo. If some terminals corrupt Chinese paths,
  set environment variable PVZ_PROJECT_ROOT to the same path instead.
#>
param(
    [string]$ProjectRoot = '',
    [switch]$Run,
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Release',
    [ValidateSet('x64', 'x86')]
    [string]$Platform = 'x64'
)

$ErrorActionPreference = 'Stop'

function Get-SlnInDir([string]$dir) {
    if (-not $dir) { return $null }
    $item = Get-ChildItem -LiteralPath $dir -Filter '*.sln' -ErrorAction SilentlyContinue |
        Select-Object -First 1
    if ($item) { return $item.FullName }
    return $null
}

# Repo root: -ProjectRoot > env PVZ_PROJECT_ROOT > cwd with .sln > script directory
if ($ProjectRoot -and (Get-SlnInDir $ProjectRoot)) {
    $root = [System.IO.Path]::GetFullPath($ProjectRoot)
} elseif ($env:PVZ_PROJECT_ROOT -and (Get-SlnInDir $env:PVZ_PROJECT_ROOT)) {
    $root = [System.IO.Path]::GetFullPath($env:PVZ_PROJECT_ROOT)
} else {
    if (Get-SlnInDir (Get-Location).Path) {
        $root = (Get-Location).Path
    } elseif ($PSCommandPath) {
        $root = [System.IO.Path]::GetDirectoryName($PSCommandPath)
    } else {
        $root = $PSScriptRoot
    }
}

$sln = Get-SlnInDir $root
if (-not $sln) {
    throw @"
Solution (.sln) not found under: $root
Fix: cd to this repo in Explorer then open PowerShell here, or set user env PVZ_PROJECT_ROOT
to the repo path, or pass -ProjectRoot from a UTF-8 terminal.
"@
}

$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path -LiteralPath $vswhere)) {
    throw 'vswhere.exe not found. Install Visual Studio Installer.'
}

$installPath = & $vswhere -latest -property installationPath 2>$null
if (-not $installPath) {
    throw 'No Visual Studio installation found. Install VS with MSVC desktop workload.'
}

$msbuild = Join-Path $installPath 'MSBuild\Current\Bin\MSBuild.exe'
if (-not (Test-Path -LiteralPath $msbuild)) {
    throw "MSBuild not found: $msbuild"
}

$msbuildPlatform = if ($Platform -eq 'x86') { 'Win32' } else { 'x64' }

Write-Host "MSBuild: $msbuild"
Write-Host "Build: $Configuration | $Platform"

& $msbuild $sln /m /v:m /p:Configuration=$Configuration /p:Platform=$msbuildPlatform
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

$exeDir = Join-Path $root (Join-Path 'x64' $Configuration)
$exe = Get-ChildItem -LiteralPath $exeDir -Filter '*.exe' -ErrorAction SilentlyContinue |
    Select-Object -First 1 -ExpandProperty FullName
if (-not $exe) {
    $exeDir2 = Join-Path $root (Join-Path $Platform $Configuration)
    $exe = Get-ChildItem -LiteralPath $exeDir2 -Filter '*.exe' -ErrorAction SilentlyContinue |
        Select-Object -First 1 -ExpandProperty FullName
}

if (-not (Test-Path -LiteralPath $exe)) {
    Write-Host "Build OK but exe not found at expected path."
    exit 0
}

Write-Host "Output: $exe"

if ($Run) {
    Write-Host 'Launching from project root (for res/)...'
    Push-Location -LiteralPath $root
    try {
        & $exe
    } finally {
        Pop-Location
    }
}
