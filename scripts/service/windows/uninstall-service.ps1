param(
    [string]$serviceName = "adam"
)

# Check if service exists
$existingService = Get-Service -Name $serviceName -ErrorAction SilentlyContinue

if ($existingService) {
    Write-Host "Stopping service '$serviceName'..."
    Stop-Service -Name $serviceName -ErrorAction SilentlyContinue
    
    Write-Host "Removing service '$serviceName'..."
    if (Get-Command Remove-Service -ErrorAction SilentlyContinue) {
        Remove-Service -Name $serviceName -ErrorAction SilentlyContinue
    } else {
        & sc.exe delete $serviceName
    }
    
    if ($?) {
        Write-Host "Service '$serviceName' uninstalled successfully!" -ForegroundColor Green
    } else {
        Write-Host "Failed to uninstall the service. Make sure you are running this script as Administrator." -ForegroundColor Red
    }
} else {
    Write-Host "Service '$serviceName' not found." -ForegroundColor Yellow
}
