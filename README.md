# ESP32 Sesame MQTT Controller with 433MHz RF Remote

[🇻🇳 Tiếng Việt](#tiếng-việt) | [🇺🇸 English](#english)

---

## English

### 📋 Project Overview

ESP32-based smart lock controller that connects to SESAME 4/5 smart locks via Bluetooth Low Energy (BLE) and provides MQTT remote control capabilities. Features 433MHz RF remote control integration using RXB6 receiver module for wireless operation.

### ✨ Features

- 🔐 **SESAME 4/5 Integration**: Full BLE communication with SESAME smart locks
- 📡 **MQTT Control**: Remote lock/unlock via MQTT commands  
- 📻 **433MHz RF Remote**: Wireless control using RXB6 receiver module
- 🔄 **Smart Toggle**: Automatic lock/unlock based on current state
- 📊 **Real-time Status**: Battery, position, and lock state monitoring
- 🌐 **WiFi Connectivity**: Remote access over internet
- 🔋 **Auto-reconnection**: Robust connection handling
- 📈 **Status Publishing**: JSON-formatted device status via MQTT

### 🛠️ Hardware Requirements

| Component | Model | Purpose |
|-----------|-------|---------|
| **Microcontroller** | ESP32 DevKit | Main controller |
| **RF Receiver** | RXB6 433MHz | Wireless remote control |
| **Smart Lock** | SESAME 4/5 | Target device |
| **Antenna** | 17cm wire | 433MHz reception |

### 🔌 Wiring Diagram

```
ESP32 DevKit          RXB6 433MHz Module
├── 3.3V         →    VCC
├── GND          →    GND
├── GPIO 32      →    DATA
└── (optional)   →    ANT (17cm wire antenna)

SESAME 4/5: Bluetooth LE connection (no wiring required)
```

### 📥 Software Setup

#### Prerequisites

**System Requirements:**
- **Python 3.8+** (recommended: Python 3.9 or later)
- **Git** for version control
- **USB Driver** for ESP32 (CP210x or CH340)

**Software Installation:**
```bash
# Install PlatformIO CLI (choose method based on your system)

# Method 1: Using pip (if available in PATH)
pip install platformio

# Method 2: Using python module (more reliable)
python -m pip install platformio
# OR for systems with both Python 2/3
python3 -m pip install platformio

# Method 3: For Windows users
py -m pip install platformio

# Verify installation
pio --version
# OR if 'pio' command not found
python -m platformio --version
# OR
python3 -m platformio --version
```

**Important Notes:**
- If `pio` command is not found, use `python -m platformio` instead
- On Windows, you might need to use `py` instead of `python`
- Add PlatformIO to PATH: `pip install platformio` should add it automatically, but if not:
  ```bash
  # Add to PATH manually (varies by OS)
  # Windows: Add C:\Users\{username}\.platformio\penv\Scripts to PATH
  # Linux/Mac: Add ~/.platformio/penv/bin to PATH
  ```

**Required Libraries** (auto-installed by PlatformIO):
- `ArduinoJson @ 7.4.2` - JSON parsing
- `PubSubClient @ 2.8.0` - MQTT client
- `libsesame3bt @ 0.25.0` - Official SESAME library
- `NimBLE-Arduino @ 2.2.3` - Bluetooth Low Energy
- `libsesame3bt-core @ 0.11.0` - SESAME core functions
- `WiFi @ 2.0.0` - ESP32 WiFi (built-in)

**Hardware:**
- ESP32 DevKit (any variant)
- RXB6 433MHz receiver module
- SESAME 4/5 smart lock
- USB cable (micro-USB or USB-C depending on ESP32 variant)
- 433MHz remote control (optional)
- **SESAME mobile app** for device registration

#### Installation

1. **Clone Repository**
   ```bash
   git clone <repository-url>
   cd esp32-sesame-controller
   ```

2. **Configure Settings**
   Edit `include/config.h`:
   ```cpp
   // WiFi Configuration
   #define WIFI_SSID "your-wifi-name"
   #define WIFI_PASSWORD "your-wifi-password"
   
   // MQTT Configuration  
   #define MQTT_SERVER "192.168.1.100"
   #define MQTT_USERNAME "your-mqtt-user"
   #define MQTT_PASSWORD "your-mqtt-pass"
   
   // SESAME Keys (from QR code in SESAME app)
   #define SESAME_SECRET "your-32-char-secret-key"
   #define SESAME_PUBLIC_KEY "your-128-char-public-key"
   #define SESAME_DEVICE_ADDRESS "xx:xx:xx:xx:xx:xx"
   ```

3. **Get SESAME Keys**
   - Open SESAME mobile app
   - Go to device settings → Share device → Generate QR code
   - Scan QR code to get Secret and Public keys
   - Find device BLE address using ESP32 scan (see QUICKSTART.md)

4. **Upload Firmware**
   ```bash
   # Primary method
   platformio run --target upload
   
   # Alternative methods if 'platformio' command not found
   python -m platformio run --target upload
   # OR
   python3 -m platformio run --target upload
   # OR (Windows)
   py -m platformio run --target upload
   ```

5. **Monitor Serial Output**
   ```bash
   # Primary method
   platformio device monitor --baud 115200
   
   # Alternative methods if 'platformio' command not found
   python -m platformio device monitor --baud 115200
   # OR
   python3 -m platformio device monitor --baud 115200
   # OR (Windows)
   py -m platformio device monitor --baud 115200
   ```

### 🔧 Development & Deployment Commands

**📝 Note**: If `pio` command is not found, replace `pio` with `python -m platformio` or `python3 -m platformio`

#### Build Commands
```bash
# Clean build directory
pio run --target clean
# Alternative: python -m platformio run --target clean
# Alternative: python3 -m platformio run --target clean

# Build project (compile only)
pio run
# Alternative: python -m platformio run

# Build with verbose output
pio run --verbose
# Alternative: python -m platformio run --verbose

# Build for specific environment
pio run --environment esp32dev
# Alternative: python -m platformio run --environment esp32dev
```

#### Upload Commands
```bash
# Upload firmware to ESP32
pio run --target upload
# Alternative: python -m platformio run --target upload

# Upload with specific port
pio run --target upload --upload-port COM9
# Alternative: python -m platformio run --target upload --upload-port COM9

# Upload and monitor in one command
pio run --target upload && pio device monitor --baud 115200
# Alternative: python -m platformio run --target upload && python -m platformio device monitor --baud 115200

# Force upload (ignore verification)
pio run --target upload --upload-port COM9 --force
# Alternative: python -m platformio run --target upload --upload-port COM9 --force
```

#### Monitoring Commands
```bash
# Basic serial monitor
pio device monitor --baud 115200
# Alternative: python -m platformio device monitor --baud 115200

# Monitor with specific port
pio device monitor --port COM9 --baud 115200
# Alternative: python -m platformio device monitor --port COM9 --baud 115200

# Monitor with filters
pio device monitor --baud 115200 --filter esp32_exception_decoder
# Alternative: python -m platformio device monitor --baud 115200 --filter esp32_exception_decoder

# Monitor with timestamp
pio device monitor --baud 115200 --filter time
# Alternative: python -m platformio device monitor --baud 115200 --filter time

# Exit monitor: Ctrl+C
```

#### Development Workflow
```bash
# Complete development cycle
pio run --target clean    # Clean previous build
pio run                   # Build firmware  
pio run --target upload   # Upload to ESP32
pio device monitor --baud 115200  # Monitor output

# Alternative complete cycle
python -m platformio run --target clean
python -m platformio run
python -m platformio run --target upload
python -m platformio device monitor --baud 115200

# Quick update cycle (most common)
pio run --target upload && pio device monitor --baud 115200
# Alternative: python -m platformio run --target upload && python -m platformio device monitor --baud 115200
```

#### Troubleshooting Commands
```bash
# List available serial ports
pio device list
# Alternative: python -m platformio device list

# Check project configuration
pio project config
# Alternative: python -m platformio project config

# Update PlatformIO libraries
pio lib update
# Alternative: python -m platformio lib update

# Check library dependencies
pio lib deps
# Alternative: python -m platformio lib deps

# Reset ESP32 via software
pio device monitor --baud 115200 --rts 1 --dtr 1
# Alternative: python -m platformio device monitor --baud 115200 --rts 1 --dtr 1
```

#### Common Command Issues & Solutions
```bash
# If you get "pio: command not found"
# Use this instead of 'pio':
python -m platformio
# or
python3 -m platformio

# If you get "pip: command not found"
# Use this instead of 'pip':
python -m pip
# or
python3 -m pip

# Check your Python installation
python --version
python3 --version

# Check if PlatformIO is installed
python -m platformio --version
```

### 🎮 Usage

#### MQTT Commands

**Topic**: `sesame/command`

**Lock Device**:
```json
{"action": "lock", "tag": "Remote control"}
```

**Unlock Device**:
```json
{"action": "unlock", "tag": "Remote control"}
```

**Get Status**:
```json
{"action": "status"}
```

#### 433MHz RF Remote

1. Connect RXB6 module as per wiring diagram
2. Press any 433MHz remote button
3. ESP32 automatically toggles lock state:
   - If **locked** → **unlocks**
   - If **unlocked** → **locks**

#### Status Monitoring

**Topic**: `sesame/status` (JSON format)

```json
{
  "device": "セサミ4",
  "address": "f9:cc:af:16:ae:dd", 
  "wifi_connected": true,
  "mqtt_connected": true,
  "sesame_connected": true,
  "sesame_authenticated": true,
  "battery_pct": 85.2,
  "voltage": 6.1,
  "position": 3,
  "locked": true,
  "unlocked": false
}
```

**RXB6 Notifications**: `sesame/rxb6`
```json
{
  "signal": "received",
  "timestamp": 1234567890,
  "pin": 32
}
```

### 🔧 Configuration Options

Edit `include/config.h` for customization:

```cpp
// RXB6 433MHz Configuration
#define RXB6_DATA_PIN 32           // GPIO pin for RXB6 DATA
#define RXB6_SIGNAL_TIMEOUT 500    // Minimum time between signals (ms)
#define RXB6_SIGNAL_MIN_WIDTH 50   // Minimum signal width (ms)
#define RXB6_ENABLED true          // Enable/disable RXB6

// SESAME Configuration
#define SESAME_MODEL_TYPE 4        // 4=SESAME 4, 5=SESAME 5
#define AUTO_TEST_ENABLED true     // Auto-test after connection
```

### 🔍 Troubleshooting

#### Connection Issues
- **SESAME app must be completely closed** during ESP32 connection
- Ensure ESP32 is within 2-3 meters of SESAME device
- Verify keys are correct (32 + 128 hex characters)
- Check BLE address matches discovered device

#### 433MHz Issues  
- Verify RXB6 wiring (especially DATA pin to GPIO 32)
- Check 433MHz remote frequency matches RXB6
- Add 17cm antenna wire for better reception
- Monitor serial output for signal detection logs

#### MQTT Issues
- Verify MQTT broker is accessible
- Check WiFi connection stability
- Confirm MQTT credentials and topics

### 📚 API Reference

#### Core Functions
- `connectToSesame()`: Establish BLE connection
- `sendSesameCommand(String)`: Execute lock/unlock/status
- `toggleSesame()`: Smart toggle based on current state
- `processRXB6Signal()`: Handle 433MHz RF signals

#### Callbacks
- `statusUpdate()`: Receive real-time status updates
- `stateUpdate()`: Monitor connection state changes  
- `rxb6InterruptHandler()`: Process RF signals (ISR)

### 🤝 Contributing

1. Fork the repository
2. Create feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit changes (`git commit -m 'Add AmazingFeature'`)
4. Push to branch (`git push origin feature/AmazingFeature`)
5. Open Pull Request

### 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## Tiếng Việt

### 📋 Tổng Quan Dự Án

Bộ điều khiển khóa thông minh dựa trên ESP32 kết nối với khóa thông minh SESAME 4/5 qua Bluetooth Low Energy (BLE) và cung cấp khả năng điều khiển từ xa qua MQTT. Tích hợp điều khiển RF 433MHz sử dụng module RXB6 để vận hành không dây.

### ✨ Tính Năng

- 🔐 **Tích hợp SESAME 4/5**: Giao tiếp BLE đầy đủ với khóa thông minh SESAME
- 📡 **Điều khiển MQTT**: Khóa/mở từ xa qua lệnh MQTT
- 📻 **Remote RF 433MHz**: Điều khiển không dây bằng module RXB6  
- 🔄 **Toggle Thông minh**: Tự động khóa/mở dựa trên trạng thái hiện tại
- 📊 **Trạng thái Real-time**: Giám sát pin, vị trí và trạng thái khóa
- 🌐 **Kết nối WiFi**: Truy cập từ xa qua internet
- 🔋 **Tự động kết nối lại**: Xử lý kết nối mạnh mẽ
- 📈 **Publish Status**: Trạng thái thiết bị định dạng JSON qua MQTT

### 🛠️ Yêu Cầu Phần Cứng

| Linh kiện | Model | Mục đích |
|-----------|-------|----------|
| **Vi điều khiển** | ESP32 DevKit | Bộ điều khiển chính |
| **RF Receiver** | RXB6 433MHz | Điều khiển từ xa không dây |
| **Khóa thông minh** | SESAME 4/5 | Thiết bị đích |
| **Anten** | Dây 17cm | Thu sóng 433MHz |

### 🔌 Sơ Đồ Đấu Dây

```
ESP32 DevKit          Module RXB6 433MHz
├── 3.3V         →    VCC
├── GND          →    GND
├── GPIO 32      →    DATA
└── (tùy chọn)   →    ANT (anten dây 17cm)

SESAME 4/5: Kết nối Bluetooth LE (không cần đấu dây)
```

### 📥 Cài Đặt Phần Mềm

#### Yêu Cầu Trước

**Yêu Cầu Hệ Thống:**
- **Python 3.8+** (khuyến nghị: Python 3.9 trở lên)
- **Git** để quản lý phiên bản
- **USB Driver** cho ESP32 (CP210x hoặc CH340)

**Cài Đặt Phần Mềm:**
```bash
# Cài đặt PlatformIO CLI
pip install platformio

# Kiểm tra cài đặt
pio --version
platformio --version
```

**Thư Viện Cần Thiết** (tự động cài đặt bởi PlatformIO):
- `ArduinoJson @ 7.4.2` - Xử lý JSON
- `PubSubClient @ 2.8.0` - MQTT client
- `libsesame3bt @ 0.25.0` - Thư viện SESAME chính thức
- `NimBLE-Arduino @ 2.2.3` - Bluetooth Low Energy
- `libsesame3bt-core @ 0.11.0` - Chức năng core SESAME
- `WiFi @ 2.0.0` - ESP32 WiFi (tích hợp sẵn)

**Phần Cứng:**
- ESP32 DevKit (bất kỳ phiên bản nào)
- Module RXB6 433MHz receiver
- Khóa thông minh SESAME 4/5
- Cáp USB (micro-USB hoặc USB-C tùy theo ESP32)
- Remote 433MHz (tùy chọn)
- **App SESAME** để đăng ký thiết bị

#### Cài Đặt

1. **Clone Repository**
   ```bash
   git clone <repository-url>
   cd esp32-sesame-controller
   ```

2. **Cấu Hình Settings**
   Chỉnh sửa `include/config.h`:
   ```cpp
   // Cấu hình WiFi
   #define WIFI_SSID "ten-wifi-cua-ban"
   #define WIFI_PASSWORD "mat-khau-wifi"
   
   // Cấu hình MQTT
   #define MQTT_SERVER "192.168.1.100"
   #define MQTT_USERNAME "mqtt-user"
   #define MQTT_PASSWORD "mqtt-password"
   
   // Khóa SESAME (từ QR code trong app SESAME)
   #define SESAME_SECRET "secret-key-32-ky-tu"
   #define SESAME_PUBLIC_KEY "public-key-128-ky-tu"
   #define SESAME_DEVICE_ADDRESS "xx:xx:xx:xx:xx:xx"
   ```

3. **Lấy Khóa SESAME**
   - Mở app SESAME trên điện thoại
   - Vào cài đặt thiết bị → Chia sẻ thiết bị → Tạo QR code
   - Scan QR code để lấy Secret và Public key
   - Tìm địa chỉ BLE bằng ESP32 scan (xem QUICKSTART.md)

4. **Upload Firmware**
   ```bash
   # Primary method
   platformio run --target upload
   
   # Alternative methods if 'platformio' command not found
   python -m platformio run --target upload
   # OR
   python3 -m platformio run --target upload
   # OR (Windows)
   py -m platformio run --target upload
   ```

5. **Monitor Serial Output**
   ```bash
   # Primary method
   platformio device monitor --baud 115200
   
   # Alternative methods if 'platformio' command not found
   python -m platformio device monitor --baud 115200
   # OR
   python3 -m platformio device monitor --baud 115200
   # OR (Windows)
   py -m platformio device monitor --baud 115200
   ```

### 🔧 Lệnh Phát Triển & Triển Khai

#### Lệnh Build
```bash
# Xóa thư mục build
pio run --target clean
# Alternative: python -m platformio run --target clean
# Alternative: python3 -m platformio run --target clean

# Build dự án (chỉ compile)
pio run
# Alternative: python -m platformio run

# Build với output chi tiết
pio run --verbose
# Alternative: python -m platformio run --verbose

# Build cho environment cụ thể
pio run --environment esp32dev
# Alternative: python -m platformio run --environment esp32dev
```

#### Lệnh Upload
```bash
# Upload firmware lên ESP32
pio run --target upload
# Alternative: python -m platformio run --target upload

# Upload với port cụ thể
pio run --target upload --upload-port COM9
# Alternative: python -m platformio run --target upload --upload-port COM9

# Upload và monitor trong một lệnh
pio run --target upload && pio device monitor --baud 115200
# Alternative: python -m platformio run --target upload && python -m platformio device monitor --baud 115200

# Ép buộc upload (bỏ qua verification)
pio run --target upload --upload-port COM9 --force
# Alternative: python -m platformio run --target upload --upload-port COM9 --force
```

#### Lệnh Monitor
```bash
# Serial monitor cơ bản
pio device monitor --baud 115200
# Alternative: python -m platformio device monitor --baud 115200

# Monitor với port cụ thể
pio device monitor --port COM9 --baud 115200
# Alternative: python -m platformio device monitor --port COM9 --baud 115200

# Monitor với filters
pio device monitor --baud 115200 --filter esp32_exception_decoder
# Alternative: python -m platformio device monitor --baud 115200 --filter esp32_exception_decoder

# Monitor với timestamp
pio device monitor --baud 115200 --filter time
# Alternative: python -m platformio device monitor --baud 115200 --filter time

# Thoát monitor: Ctrl+C
```

#### Quy Trình Phát Triển
```bash
# Chu trình phát triển hoàn chỉnh
pio run --target clean    # Xóa build cũ
pio run                   # Build firmware  
pio run --target upload   # Upload lên ESP32
pio device monitor --baud 115200  # Monitor output

# Alternative complete cycle
python -m platformio run --target clean
python -m platformio run
python -m platformio run --target upload
python -m platformio device monitor --baud 115200

# Quick update cycle (most common)
pio run --target upload && pio device monitor --baud 115200
# Alternative: python -m platformio run --target upload && python -m platformio device monitor --baud 115200
```

#### Lệnh Khắc Phục Sự Cố
```bash
# Liệt kê các cổng serial
pio device list
# Alternative: python -m platformio device list

# Kiểm tra cấu hình dự án
pio project config
# Alternative: python -m platformio project config

# Cập nhật thư viện PlatformIO
pio lib update
# Alternative: python -m platformio lib update

# Kiểm tra dependencies thư viện
pio lib deps
# Alternative: python -m platformio lib deps

# Reset ESP32 qua phần mềm
pio device monitor --baud 115200 --rts 1 --dtr 1
# Alternative: python -m platformio device monitor --baud 115200 --rts 1 --dtr 1
```

### 🎮 Sử Dụng

#### Lệnh MQTT

**Topic**: `sesame/command`

**Khóa Thiết Bị**:
```json
{"action": "lock", "tag": "Điều khiển từ xa"}
```

**Mở Thiết Bị**:
```json
{"action": "unlock", "tag": "Điều khiển từ xa"}
```

**Lấy Trạng Thái**:
```json
{"action": "status"}
```

#### Remote RF 433MHz

1. Kết nối module RXB6 theo sơ đồ đấu dây
2. Nhấn bất kỳ nút remote 433MHz nào
3. ESP32 tự động toggle trạng thái khóa:
   - Nếu **đang khóa** → **mở khóa**
   - Nếu **đang mở** → **khóa lại**

#### Giám Sát Trạng Thái

**Topic**: `sesame/status` (định dạng JSON)

```json
{
  "device": "セサミ4",
  "address": "f9:cc:af:16:ae:dd",
  "wifi_connected": true,
  "mqtt_connected": true, 
  "sesame_connected": true,
  "sesame_authenticated": true,
  "battery_pct": 85.2,
  "voltage": 6.1,
  "position": 3,
  "locked": true,
  "unlocked": false
}
```

**Thông báo RXB6**: `sesame/rxb6`
```json
{
  "signal": "received",
  "timestamp": 1234567890,
  "pin": 32
}
```

### 🔧 Tùy Chọn Cấu Hình

Chỉnh sửa `include/config.h` để tùy chỉnh:

```cpp
// Cấu hình RXB6 433MHz
#define RXB6_DATA_PIN 32           // Chân GPIO cho RXB6 DATA
#define RXB6_SIGNAL_TIMEOUT 500    // Thời gian tối thiểu giữa các tín hiệu (ms)
#define RXB6_SIGNAL_MIN_WIDTH 50   // Độ rộng tín hiệu tối thiểu (ms)
#define RXB6_ENABLED true          // Bật/tắt RXB6

// Cấu hình SESAME
#define SESAME_MODEL_TYPE 4        // 4=SESAME 4, 5=SESAME 5  
#define AUTO_TEST_ENABLED true     // Tự động test sau khi kết nối
```

### 🔍 Khắc Phục Sự Cố

#### Sự Cố Kết Nối
- **App SESAME phải đóng hoàn toàn** trong quá trình ESP32 kết nối
- Đảm bảo ESP32 trong phạm vi 2-3 mét từ thiết bị SESAME
- Xác minh khóa đúng (32 + 128 ký tự hex)
- Kiểm tra địa chỉ BLE khớp với thiết bị được phát hiện

#### Sự Cố 433MHz
- Xác minh đấu dây RXB6 (đặc biệt chân DATA tới GPIO 32)
- Kiểm tra tần số remote 433MHz khớp với RXB6
- Thêm anten dây 17cm để thu sóng tốt hơn
- Giám sát serial output để xem log phát hiện tín hiệu

#### Sự Cố MQTT
- Xác minh MQTT broker có thể truy cập
- Kiểm tra kết nối WiFi ổn định
- Xác nhận thông tin đăng nhập MQTT và topics

### 📚 Tham Khảo API

#### Hàm Chính
- `connectToSesame()`: Thiết lập kết nối BLE
- `sendSesameCommand(String)`: Thực thi lock/unlock/status
- `toggleSesame()`: Toggle thông minh dựa trên trạng thái hiện tại
- `processRXB6Signal()`: Xử lý tín hiệu RF 433MHz

#### Callbacks
- `statusUpdate()`: Nhận cập nhật trạng thái real-time
- `stateUpdate()`: Giám sát thay đổi trạng thái kết nối
- `rxb6InterruptHandler()`: Xử lý tín hiệu RF (ISR)

### 🤝 Đóng Góp

1. Fork repository
2. Tạo feature branch (`git checkout -b feature/TinhNangTuyetVoi`)
3. Commit thay đổi (`git commit -m 'Thêm TinhNangTuyetVoi'`)
4. Push lên branch (`git push origin feature/TinhNangTuyetVoi`)
5. Mở Pull Request

### 📄 Giấy Phép

Dự án này được cấp phép theo MIT License - xem file [LICENSE](LICENSE) để biết chi tiết.

---

## 📞 Support & Contact

- **Issues**: [GitHub Issues](https://github.com/your-repo/issues)
- **Documentation**: [Wiki](https://github.com/your-repo/wiki)
- **Community**: [Discussions](https://github.com/your-repo/discussions)

## 🙏 Acknowledgments

- [libsesame3bt](https://github.com/homy-newfs8/libsesame3bt) - Official SESAME 3/4/5 library
- [PlatformIO](https://platformio.org/) - Development platform
- [SESAME](https://candyhouse.co/) - Smart lock hardware 