# 🚀 ESP32 Sesame MQTT Controller - Quick Start Guide

This project uses the **official libsesame3bt library** to control Sesame smart locks via MQTT commands.

## ⚠️ **IMPORTANT: Changed Approach!**

**This version requires REGISTERED devices**, not unregistered ones. You MUST:
1. **Register your Sesame device via the official SESAME app first**
2. **Get both Public Key and Secret Key from QR code**
3. **Device must show as 'registered' in scan results**

## 📋 **Prerequisites**

### 1. Hardware Requirements
- **ESP32 development board** (tested on ESP32-WROOM-32)
- **Sesame smart lock** (Sesame 3, 4, 5, 5 Pro supported)
- **WiFi network** (2.4GHz)
- **MQTT broker** running

### 2. Device Registration (CRITICAL STEP!)

#### Step 1: Register via SESAME App
1. **Download** official SESAME app on your phone
2. **Add your device** through the app (complete setup process)
3. **Verify** the device works via app (lock/unlock)
4. **Keep the device** connected to your account

#### Step 2: Get Keys from QR Code
1. **Open SESAME app** → Select your device
2. **Go to Settings** → Device Settings → QR Code
3. **Scan/View the QR code** - it contains:
   - **Device UUID** (128-bit)
   - **Public Key** (128 hex characters)
   - **Secret Key** (32 hex characters)
4. **Copy both keys** - you need BOTH for ESP32

## 🔧 **Configuration**

### 1. WiFi & MQTT Settings
Edit `include/config.h`:

```cpp
// WiFi Configuration
#define WIFI_SSID "your_wifi_name"
#define WIFI_PASSWORD "your_wifi_password"

// MQTT Configuration
#define MQTT_SERVER "192.168.1.100"  // Your MQTT broker IP
#define MQTT_PORT 1883
#define MQTT_USERNAME "your_mqtt_user"
#define MQTT_PASSWORD "your_mqtt_password"
```

### 2. Sesame Device Configuration
**MOST IMPORTANT STEP!** Edit `include/config.h`:

```cpp
// Get these from SESAME app QR code
#define SESAME_SECRET "1234567890abcdef1234567890abcdef"  // 32 hex chars
#define SESAME_PUBLIC_KEY "1234567890abcdef..."         // 128 hex chars

// Device model (change based on your device)
#define SESAME_MODEL_TYPE 5  // 5=Sesame 5, 4=Sesame 4, 3=Sesame 3

// Device name for identification
#define SESAME_DEVICE_NAME "front door"
```

## 🔨 **Build & Upload**

### Method 1: PlatformIO CLI
```bash
# Install dependencies and build
pio lib install
pio run

# Upload to ESP32
pio run --target upload

# Monitor serial output
pio device monitor --baud 115200
```

### Method 2: PlatformIO IDE
1. **Open project** in VS Code with PlatformIO extension
2. **Build project** (Ctrl+Alt+B)
3. **Upload** (Ctrl+Alt+U)
4. **Monitor** (Ctrl+Alt+S)

## 📊 **Expected Startup Logs**

### ✅ Successful Connection:
```
=== ESP32 Sesame MQTT Controller (Official libsesame3bt) ===
📱 Target: front door
🎯 Target model: SESAME 5

⚠️  IMPORTANT: Your Sesame device MUST be:
   1. Registered via official SESAME app first
   2. QR code scanned to get both Public Key and Secret Key
   3. Device shown as 'registered' in scan results
   4. Not connected to the app during ESP32 operation

✅ WiFi connected: 192.168.1.150
✅ MQTT connected
✅ BLE initialized

🔍 Scanning for registered Sesame devices...
🎯 Found: f9:cc:af:16:ae:dd, model=SESAME 5, registered=1
📋 Scan completed: 1 devices found
✅ Found registered Sesame: f9:cc:af:16:ae:dd (SESAME 5)
🔗 Connecting to Sesame device...
✅ Connected to Sesame!
🔄 Sesame state: connected
🔄 Sesame state: authenticating
🔄 Sesame state: active
⚡ Starting auto-test sequence...
🔓 Auto-test: UNLOCK
```

## 🎮 **MQTT Commands**

### Control Commands
Send to topic: `sesame/command`

**Unlock:**
```json
{"action": "unlock"}
```

**Lock:**
```json
{"action": "lock"}
```

**Get Status:**
```json
{"action": "status"}
```

**Rescan Devices:**
```json
{"action": "scan"}
```

### Status Monitoring
Subscribe to topic: `sesame/status`

**Example status:**
```json
{
  "wifi_connected": true,
  "mqtt_connected": true,
  "sesame_connected": true,
  "sesame_authenticated": true,
  "device_address": "f9:cc:af:16:ae:dd",
  "device_name": "SESAME 5",
  "auto_test_completed": true,
  "lock_status": "locked",
  "battery_percent": 85.5,
  "battery_voltage": 3.2,
  "position": 512
}
```

## 🔧 **Troubleshooting**

### ❌ "No suitable registered Sesame devices found"

**Solutions:**
1. **Verify registration**: Device must be added to SESAME app account
2. **Check power**: Ensure device is powered on and nearby
3. **Close SESAME app**: Don't use app while ESP32 is connecting
4. **Check model type**: Verify `SESAME_MODEL_TYPE` matches your device

### ❌ "Failed to set keys" or Authentication fails

**Solutions:**
1. **Re-scan QR code**: Get fresh keys from SESAME app
2. **Verify format**: 
   - Secret: exactly 32 hex characters
   - Public: exactly 128 hex characters
3. **Check for typos**: Copy-paste carefully, no spaces

---
**Important:** This version requires properly registered devices via SESAME app first.
