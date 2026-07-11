param(
    [string]$exePath = "$PSScriptRoot\adam.exe",
    [string]$serviceName = "AdamService",
    [string]$displayName = "Adam Application Service",
    [string]$description = "Runs the Adam application in the background"
)

if (-not (Test-Path $exePath)) {
    Write-Host "Warning: Executable not found at $exePath." -ForegroundColor Yellow
    Write-Host "Please provide the correct path using the -exePath parameter." -ForegroundColor Yellow
    Write-Host "Example: .\install-service.ps1 -exePath `"C:\path\to\adam.exe`""
} else {
    $exePath = (Resolve-Path $exePath).Path
}

# Check if service already exists
$existingService = Get-Service -Name $serviceName -ErrorAction SilentlyContinue
if ($existingService) {
    Write-Host "Service '$serviceName' already exists. Stopping and removing it first..."
    Stop-Service -Name $serviceName -ErrorAction SilentlyContinue
    # Remove-Service is available in PowerShell 6+, or we can use sc.exe
    if (Get-Command Remove-Service -ErrorAction SilentlyContinue) {
        Remove-Service -Name $serviceName -ErrorAction SilentlyContinue
    } else {
        & sc.exe delete $serviceName
    }
    Start-Sleep -Seconds 2
}

Write-Host "Installing service '$serviceName'..."
New-Service -Name $serviceName -BinaryPathName $exePath -DisplayName $displayName -Description $description -StartupType Automatic

if ($?) {
    Write-Host "Starting service..."
    Start-Service -Name $serviceName
    Write-Host "Service installed and started successfully!" -ForegroundColor Green
} else {
    Write-Host "Failed to install the service. Make sure you are running this script as Administrator." -ForegroundColor Red
}
