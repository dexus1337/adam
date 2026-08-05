# adam-cop Map Tile Pre-Fetch Utility (PowerShell Wrapper)
Param(
    [int]$MinZoom = 0,
    [int]$MaxZoom = 7,
    [string]$Provider = "all",
    [int]$Threads = 8,
    [string]$OutputDir = ".\map_cache"
)

$scriptPath = Join-Path $PSScriptRoot "prefetch_map_tiles.py"

if (-not (Test-Path $scriptPath)) {
    Write-Error "Python prefetch script not found at $scriptPath"
    exit 1
}

Write-Host "Executing adam-cop tile pre-fetch script..." -ForegroundColor Cyan
python $scriptPath --min-zoom $MinZoom --max-zoom $MaxZoom --provider $Provider --threads $Threads --output-dir $OutputDir
