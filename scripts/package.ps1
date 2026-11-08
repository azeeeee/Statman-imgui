<#
.SYNOPSIS
    Stages a clean Statman release folder from the Release build output.

.DESCRIPTION
    Copies only the files that are loaded at runtime into
    _build\dist\Statman-<version>. Debug symbols, XML documentation files,
    logs and personal configuration are never included.

.PARAMETER Configuration
    Build configuration to package. Defaults to Release.

.PARAMETER Zip
    Also write _build\dist\Statman-<version>.zip.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File scripts\package.ps1 -Zip
#>
param(
    [string]$Configuration = "Release",
    [switch]$Zip
)

$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
$buildDirectory = Join-Path $root "_build\bin32\$Configuration"
$executable = Join-Path $buildDirectory "Statman.exe"

if (-not (Test-Path $executable)) {
    throw "Build the solution first: '$executable' was not found."
}

$version = (Get-Item $executable).VersionInfo.FileVersion
$distDirectory = Join-Path $root "_build\dist\Statman-$version"

if (Test-Path $distDirectory) {
    Remove-Item $distDirectory -Recurse -Force
}

New-Item -ItemType Directory -Path $distDirectory -Force | Out-Null

# Application, its binding redirects and the in-game module.
Copy-Item $executable $distDirectory
Copy-Item (Join-Path $buildDirectory "Statman.exe.config") $distDirectory
Copy-Item (Join-Path $buildDirectory "HM3.dll") $distDirectory

# Managed dependencies. HM3.dll is the only native one and is copied above.
Get-ChildItem $buildDirectory -Filter *.dll |
    Where-Object { $_.Name -ne "HM3.dll" } |
    Copy-Item -Destination $distDirectory

# Empty default for the Hitman 2016 locations. The source template is used so
# a locations file edited in the build folder is never shipped by accident.
$locationsTemplate = Join-Path $root "Statman\HM3_locations.json"

if (Test-Path $locationsTemplate) {
    New-Item -ItemType Directory -Path (Join-Path $distDirectory "config") -Force | Out-Null
    Copy-Item $locationsTemplate (Join-Path $distDirectory "config\HM3_locations.json")
}

Write-Host "Release folder: $distDirectory"

if ($Zip) {
    $zipPath = Join-Path $root "_build\dist\Statman-$version.zip"

    if (Test-Path $zipPath) {
        Remove-Item $zipPath -Force
    }

    Compress-Archive -Path (Join-Path $distDirectory "*") -DestinationPath $zipPath
    Write-Host "Archive:        $zipPath"
}
