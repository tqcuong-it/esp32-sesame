#!/bin/bash

# ESP32 Sesame 3 MQTT Controller Build Script
# Sử dụng: ./scripts/build.sh [command]

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if PlatformIO is installed
check_platformio() {
    if ! command -v pio &> /dev/null; then
        print_error "PlatformIO not found!"
        echo "Please install PlatformIO:"
        echo "  pip install platformio"
        echo "  OR install VS Code PlatformIO extension"
        exit 1
    fi
    print_success "PlatformIO found: $(pio --version)"
}

# Check configuration
check_config() {
    print_status "Checking configuration..."
    
    if [ ! -f "include/config.h" ]; then
        print_error "Configuration file not found: include/config.h"
        print_warning "Please copy and configure include/config.h"
        exit 1
    fi
    
    # Check if default values are still in use
    if grep -q "YOUR_WIFI_SSID" include/config.h; then
        print_warning "Please update WiFi settings in include/config.h"
    fi
    
    if grep -q "YOUR_MQTT_SERVER" include/config.h; then
        print_warning "Please update MQTT settings in include/config.h"
    fi
    
    if grep -q "AB:CD:EF:01:02:03" include/config.h; then
        print_warning "Please update Sesame 3 MAC address in include/config.h"
    fi
    
    if grep -q "0123456789abcdef0123456789abcdef" include/config.h; then
        print_warning "Please update Sesame 3 secret key in include/config.h"
    fi
    
    print_success "Configuration file exists"
}

# Clean build files
clean() {
    print_status "Cleaning build files..."
    pio run --target clean
    rm -rf .pio/build
    print_success "Clean completed"
}

# Build project
build() {
    print_status "Building project..."
    check_platformio
    check_config
    
    pio run
    
    if [ $? -eq 0 ]; then
        print_success "Build completed successfully!"
    else
        print_error "Build failed!"
        exit 1
    fi
}

# Upload to ESP32
upload() {
    print_status "Uploading to ESP32..."
    check_platformio
    
    # Check if ESP32 is connected
    if ! pio device list | grep -q "USB"; then
        print_warning "No USB devices found. Please connect ESP32."
        echo "Available devices:"
        pio device list
        exit 1
    fi
    
    pio run --target upload
    
    if [ $? -eq 0 ]; then
        print_success "Upload completed successfully!"
    else
        print_error "Upload failed!"
        exit 1
    fi
}

# Monitor serial output
monitor() {
    print_status "Starting serial monitor..."
    check_platformio
    
    print_status "Press Ctrl+C to exit monitor"
    pio device monitor --baud 115200
}

# Build and upload
deploy() {
    print_status "Deploying (build + upload)..."
    build
    upload
    print_success "Deployment completed!"
}

# Full deployment with monitoring
full_deploy() {
    print_status "Full deployment (build + upload + monitor)..."
    build
    upload
    print_status "Starting monitor in 3 seconds..."
    sleep 3
    monitor
}

# Show help
show_help() {
    echo "ESP32 Sesame 3 MQTT Controller Build Script"
    echo "Usage: $0 [command]"
    echo ""
    echo "Commands:"
    echo "  build      - Build the project"
    echo "  upload     - Upload to ESP32"
    echo "  monitor    - Monitor serial output"
    echo "  deploy     - Build and upload"
    echo "  full       - Build, upload and monitor"
    echo "  clean      - Clean build files"
    echo "  check      - Check configuration"
    echo "  scan       - Scan for Sesame devices"
    echo "  restore    - Restore main.cpp after scanning"
    echo "  help       - Show this help"
    echo ""
    echo "Examples:"
    echo "  $0 build"
    echo "  $0 deploy"
    echo "  $0 full"
    echo "  $0 scan"
}

# Check dependencies
check_deps() {
    print_status "Checking dependencies..."
    
    # Check PlatformIO
    check_platformio
    
    # Check project structure
    if [ ! -f "platformio.ini" ]; then
        print_error "platformio.ini not found! Are you in the project directory?"
        exit 1
    fi
    
    if [ ! -f "src/main.cpp" ]; then
        print_error "src/main.cpp not found!"
        exit 1
    fi
    
    print_success "Dependencies check passed"
}

# Scan for Sesame devices
scan_sesame() {
    print_status "Setting up Sesame device scanner..."
    
    check_platformio
    
    # Check if scanner tool exists
    if [ ! -f "tools/scan_sesame.cpp" ]; then
        print_error "Scanner tool not found: tools/scan_sesame.cpp"
        exit 1
    fi
    
    # Backup main.cpp if not already backed up
    if [ ! -f "src/main.cpp.backup" ]; then
        print_status "Backing up original main.cpp..."
        cp "src/main.cpp" "src/main.cpp.backup"
        print_success "Backup created: src/main.cpp.backup"
    else
        print_warning "Backup already exists: src/main.cpp.backup"
    fi
    
    # Copy scanner code
    print_status "Setting up scanner code..."
    cp "tools/scan_sesame.cpp" "src/main.cpp"
    print_success "Scanner code copied to src/main.cpp"
    
    # Build and upload
    print_status "Building scanner..."
    pio run
    if [ $? -ne 0 ]; then
        print_error "Build failed!"
        exit 1
    fi
    print_success "Build completed!"
    
    print_status "Uploading to ESP32..."
    pio run --target upload
    if [ $? -ne 0 ]; then
        print_error "Upload failed!"
        exit 1
    fi
    print_success "Upload completed!"
    
    # Start monitoring
    echo ""
    echo "============================================================"
    print_success "🚀 SESAME DEVICE SCANNER STARTED"
    echo "============================================================"
    echo ""
    print_warning "📱 Instructions:"
    echo "1. Make sure your Sesame device is powered on"
    echo "2. Close SESAME app on your phone"
    echo "3. Wait for scan results below"
    echo "4. Copy MAC address when found"
    echo "5. Press Ctrl+C to stop monitoring"
    echo ""
    print_status "📊 Serial Monitor Output:"
    echo "============================================================"
    
    # Start serial monitor
    pio device monitor --baud 115200
    
    echo ""
    print_warning "🎯 Next Steps:"
    echo "1. Note down the MAC address from scanner output"
    echo "2. Get SECRET key from SESAME app QR code"
    echo "3. Update include/config.h with your values"
    echo "4. Run: $0 restore"
    echo "5. Build and upload main project"
}

# Restore main.cpp after scanning
restore_main() {
    print_status "Restoring original main.cpp..."
    
    if [ ! -f "src/main.cpp.backup" ]; then
        print_error "No backup found: src/main.cpp.backup"
        print_error "Original main.cpp was not backed up!"
        exit 1
    fi
    
    # Restore original file
    cp "src/main.cpp.backup" "src/main.cpp"
    print_success "Original main.cpp restored"
    
    # Clean up backup
    rm "src/main.cpp.backup"
    print_success "Backup file cleaned up"
    
    echo ""
    print_success "🎉 Ready to build main project!"
    echo "Run: $0 deploy"
}

# Main script logic
main() {
    # Change to project directory if script is run from scripts folder
    if [ "$(basename "$PWD")" = "scripts" ]; then
        cd ..
    fi
    
    print_status "ESP32 Sesame 3 MQTT Controller Build Script"
    print_status "Working directory: $PWD"
    
    case "${1:-help}" in
        "build")
            build
            ;;
        "upload")
            upload
            ;;
        "monitor")
            monitor
            ;;
        "deploy")
            deploy
            ;;
        "full")
            full_deploy
            ;;
        "clean")
            clean
            ;;
        "check")
            check_deps
            check_config
            ;;
        "scan")
            scan_sesame
            ;;
        "restore")
            restore_main
            ;;
        "help"|"--help"|"-h")
            show_help
            ;;
        *)
            print_error "Unknown command: $1"
            show_help
            exit 1
            ;;
    esac
}

# Run main function with all arguments
main "$@" 