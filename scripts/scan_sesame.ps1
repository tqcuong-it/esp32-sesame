# ESP32 Sesame Device Scanner Script (PowerShell)
param(
    [switch]$Help,
    [switch]$Scan,
    [switch]$Restore
)

function Write-Status {
    param([string]$Message)
    Write-Host "[INFO] $Message" -ForegroundColor Cyan
}

function Write-Success {
    param([string]$Message)
    Write-Host "[SUCCESS] $Message" -ForegroundColor Green
}

function Write-Warning {
    param([string]$Message)
    Write-Host "[WARNING] $Message" -ForegroundColor Yellow
}

function Write-Error {
    param([string]$Message)
    Write-Host "[ERROR] $Message" -ForegroundColor Red
}

function Show-Help {
    Write-Host "ESP32 Sesame Device Scanner Script" -ForegroundColor Cyan
    Write-Host "Usage: .\scripts\scan_sesame.ps1 [options]"
    Write-Host ""
    Write-Host "Options:"
    Write-Host "  -Scan      Run Sesame device scanner"
    Write-Host "  -Restore   Restore original main.cpp"
    Write-Host "  -Help      Show this help"
    Write-Host ""
    Write-Host "Examples:"
    Write-Host "  .\scripts\scan_sesame.ps1 -Scan"
    Write-Host "  .\scripts\scan_sesame.ps1 -Restore"
}

function Test-PlatformIO {
    try {
        $version = python3 -m platformio --version 2>$null
        Write-Success "PlatformIO found: $version"
        return $true
    }
    catch {
        Write-Error "PlatformIO not found!"
        Write-Host "Please install PlatformIO:"
        Write-Host "  Option 1: python3 -m pip install platformio"
        Write-Host "  Option 2: Install VS Code PlatformIO extension"
        return $false
    }
}

function Test-ProjectStructure {
    if (-not (Test-Path "platformio.ini")) {
        Write-Error "platformio.ini not found! Are you in the project directory?"
        return $false
    }
    
    if (-not (Test-Path "src\main.cpp")) {
        Write-Error "src\main.cpp not found!"
        return $false
    }
    
    if (-not (Test-Path "tools\scan_sesame.cpp")) {
        Write-Error "tools\scan_sesame.cpp not found!"
        return $false
    }
    
    return $true
}

function Start-SesameScanner {
    Write-Status "Starting Sesame Device Scanner..."
    
    if (-not (Test-PlatformIO)) {
        return $false
    }
    
    if (-not (Test-ProjectStructure)) {
        return $false
    }
    
    # Backup main.cpp if not already backed up
    if (-not (Test-Path "src\main.cpp.backup")) {
        Write-Status "Backing up original main.cpp..."
        Copy-Item "src\main.cpp" "src\main.cpp.backup"
        Write-Success "Backup created: src\main.cpp.backup"
    } else {
        Write-Warning "Backup already exists: src\main.cpp.backup"
    }
    
    # Copy scanner code
    Write-Status "Setting up scanner code..."
    Copy-Item "tools\scan_sesame.cpp" "src\main.cpp" -Force
    Write-Success "Scanner code copied to src\main.cpp"
    
    # Build and upload
    Write-Status "Building scanner..."
    $buildResult = & python3 -m platformio run 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Build failed!"
        Write-Host $buildResult
        return $false
    }
    Write-Success "Build completed!"
    
    Write-Status "Uploading to ESP32..."
    $uploadResult = & python3 -m platformio run --target upload 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Upload failed!"
        Write-Host $uploadResult
        return $false
    }
    Write-Success "Upload completed!"
    
    # Start monitoring
    Write-Host ""
    Write-Host "============================================================"
    Write-Host "SESAME DEVICE SCANNER STARTED" -ForegroundColor Green
    Write-Host "============================================================"
    Write-Host ""
    Write-Host "Instructions:" -ForegroundColor Yellow
    Write-Host "1. Make sure your Sesame device is powered on"
    Write-Host "2. Close SESAME app on your phone"
    Write-Host "3. Wait for scan results below"
    Write-Host "4. Copy MAC address when found"
    Write-Host "5. Press Ctrl+C to stop monitoring"
    Write-Host ""
    Write-Host "Serial Monitor Output:" -ForegroundColor Cyan
    Write-Host "============================================================"
    
    # Start serial monitor
    try {
        & python3 -m platformio device monitor --baud 115200
    }
    catch {
        Write-Warning "Serial monitor stopped"
    }
    
    Write-Host ""
    Write-Host "Next Steps:" -ForegroundColor Yellow
    Write-Host "1. Note down the MAC address from scanner output"
    Write-Host "2. Get SECRET key from SESAME app QR code"
    Write-Host "3. Update include\config.h with your values"
    Write-Host "4. Run: .\scripts\scan_sesame.ps1 -Restore"
    Write-Host "5. Build and upload main project"
    
    return $true
}

function Restore-MainCode {
    Write-Status "Restoring original main.cpp..."
    
    if (-not (Test-Path "src\main.cpp.backup")) {
        Write-Error "No backup found: src\main.cpp.backup"
        Write-Host "Original main.cpp was not backed up!"
        return $false
    }
    
    # Restore original file
    Copy-Item "src\main.cpp.backup" "src\main.cpp" -Force
    Write-Success "Original main.cpp restored"
    
    # Clean up
    Remove-Item "src\main.cpp.backup" -Force
    Write-Success "Backup file cleaned up"
    
    Write-Host ""
    Write-Host "Ready to build main project!" -ForegroundColor Green
    Write-Host "Run: python3 -m platformio run --target upload"
    
    return $true
}

# Main script logic
function Main {
    # Change to project root if running from scripts folder
    if ((Get-Location).Path.EndsWith("scripts")) {
        Set-Location ".."
    }
    
    Write-Host "ESP32 Sesame Device Scanner (PowerShell)" -ForegroundColor Cyan
    Write-Host "Working Directory: $(Get-Location)" -ForegroundColor Cyan
    Write-Host ""
    
    if ($Help) {
        Show-Help
        return
    }
    
    if ($Scan) {
        $result = Start-SesameScanner
        if (-not $result) {
            Write-Error "Scanner failed to start"
            exit 1
        }
        return
    }
    
    if ($Restore) {
        $result = Restore-MainCode
        if (-not $result) {
            Write-Error "Failed to restore main code"
            exit 1
        }
        return
    }
    
    # No parameters provided
    Write-Warning "No action specified"
    Show-Help
}

# Run main function
Main 