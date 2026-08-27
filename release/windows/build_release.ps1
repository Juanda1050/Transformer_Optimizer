$ErrorActionPreference = "Stop"

$RepoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$BuildDir = Join-Path $RepoRoot "build-windows"
$OutputDir = Join-Path $PSScriptRoot "bin"

New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null

cmake -S $RepoRoot -B $BuildDir -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build $BuildDir --config Release

Copy-Item (Join-Path $BuildDir "bin\transformer_optimizer.exe") $OutputDir -Force
Copy-Item (Join-Path $RepoRoot "results.csv") $OutputDir -Force

Write-Host "Windows release generated in: $OutputDir"
