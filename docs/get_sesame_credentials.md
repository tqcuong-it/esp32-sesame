# Hướng dẫn lấy SESAME_ADDRESS và SESAME_SECRET

Để kết nối ESP32 với Sesame 3, bạn cần 2 thông số quan trọng:
- **SESAME_ADDRESS**: Địa chỉ MAC Bluetooth của Sesame 3
- **SESAME_SECRET**: Khóa bí mật để xác thực và mã hóa

## 📋 Yêu cầu

- Smartphone (Android/iOS)
- App SESAME official đã cài đặt
- Sesame 3 đã được setup và hoạt động bình thường
- Bluetooth scanner app (optional)

---

## 🔍 Phương pháp 1: Lấy SESAME_SECRET từ QR Code (Khuyến nghị)

### Bước 1: Mở app SESAME trên điện thoại

1. Mở app **SESAME** trên điện thoại
2. Đăng nhập tài khoản của bạn
3. Chọn device Sesame 3 cần lấy thông tin

### Bước 2: Truy cập phần Keys

1. Trong màn hình device, tìm và tap vào **"Settings"** hoặc biểu tượng ⚙️
2. Tìm mục **"Keys"** hoặc **"Manage Keys"**
3. Tap vào **"Keys"**

### Bước 3: Hiển thị QR Code

1. Bạn sẽ thấy danh sách các keys: **Owner**, **Manager**, **Guest**
2. Chọn **"Owner"** hoặc **"Manager"** (Owner có quyền cao nhất)
3. Tap vào **"Show QR Code"** hoặc **"QR"**
4. App sẽ hiển thị QR code chứa thông tin xác thực

### Bước 4: Decode QR Code

#### Cách 1: Sử dụng tool online (Dễ nhất)

1. Truy cập: https://sesame-qr-reader.vercel.app/
2. Upload ảnh QR code hoặc scan trực tiếp
3. Tool sẽ hiển thị:
   - **Secret Key**: Đây chính là `SESAME_SECRET`
   - **Public Key**: Cần thiết cho Sesame 3/4 (để trống cho Sesame 5)
   - **Model**: Loại device

#### Cách 2: Decode manual

1. Chụp ảnh QR code
2. Sử dụng QR scanner để decode, kết quả sẽ là chuỗi URI như:
   ```
   ssm://UI?t=sk&sk=BQECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIjJCUm&l=1&n=セサミ 5
   ```

3. Lấy giá trị `sk` parameter và decode base64:
   ```bash
   # Ví dụ với sk=BQECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIjJCUm
   echo "BQECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIjJCUm" | base64 -d | xxd
   ```

4. Kết quả hex:
   ```
   00000000  05 01 02 03 04 05 06 07  08 09 0a 0b 0c 0d 0e 0f  |................|
   00000010  10 11 12 13 14 15 16 17  18 19 1a 1b 1c 1d 1e 1f  |................|
   00000020  20 21 22 23 24 25 26                              | !"#$%&|
   ```

5. Lấy 16 bytes từ vị trí thứ 2 (bỏ byte đầu):
   - **SESAME_SECRET**: `0102030405060708090a0b0c0d0e0f10`

---

## 📱 Phương pháp 2: Lấy SESAME_ADDRESS (MAC Address)

### Cách 1: Sử dụng ESP32 để scan

1. **Tạo file scan tạm thời** `src/scan_sesame.cpp`:

```cpp
#include <Arduino.h>
#include <libsesame3bt.h>

using namespace libsesame3bt;

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    
    Serial.println("=== Scanning for Sesame devices ===");
}

void loop() {
    static unsigned long lastScan = 0;
    
    if (millis() - lastScan > 10000) { // Scan every 10 seconds
        lastScan = millis();
        
        Serial.println("Scanning...");
        
        SesameScanner& scanner = SesameScanner::get();
        std::vector<SesameInfo> results;
        
        scanner.scan(5000, [&results](SesameScanner& _scanner, const SesameInfo* _info) {
            if (_info) {
                results.push_back(*_info);
            }
        });
        
        Serial.printf("Found %d Sesame devices:\n", results.size());
        
        for (const auto& info : results) {
            Serial.printf("  MAC: %s\n", info.address.toString().c_str());
            Serial.printf("  UUID: %s\n", info.uuid.toString().c_str());
            Serial.printf("  Model: %d\n", (int)info.model);
            Serial.printf("  Registered: %s\n", info.flags.registered ? "Yes" : "No");
            Serial.println("  ---");
        }
        
        if (results.empty()) {
            Serial.println("No Sesame devices found!");
            Serial.println("Make sure Sesame is powered on and nearby.");
        }
    }
    
    delay(1000);
}
```

2. **Upload và monitor**:
```bash
# Rename main.cpp tạm thời
mv src/main.cpp src/main.cpp.backup
mv src/scan_sesame.cpp src/main.cpp

# Build và upload
pio run --target upload

# Monitor kết quả
pio device monitor --baud 115200
```

3. **Kết quả sẽ hiển thị**:
```
Found 1 Sesame devices:
  MAC: AB:CD:EF:12:34:56
  UUID: 12345678-1234-1234-1234-123456789abc
  Model: 5
  Registered: Yes
  ---
```

4. **Lấy MAC address** từ kết quả: `AB:CD:EF:12:34:56`

### Cách 2: Sử dụng Bluetooth Scanner trên điện thoại

1. **Tải app Bluetooth Scanner**:
   - Android: "nRF Connect" hoặc "Bluetooth Scanner"
   - iOS: "LightBlue Explorer"

2. **Scan Bluetooth devices**:
   - Mở app và scan
   - Tìm device có tên chứa "SESAME" hoặc "CHSesame"
   - Note lại MAC address

3. **Xác nhận device**:
   - Kiểm tra UUID service có chứa: `0000fd81-0000-1000-8000-00805f9b34fb`
   - Đây là UUID đặc trưng của Sesame devices

### Cách 3: Từ SESAME app (Nếu có thông tin)

1. Vào app SESAME
2. Chọn device
3. Vào Settings/About
4. Tìm thông tin "MAC Address" hoặc "Bluetooth Address"

---

## 🔧 Cập nhật config.h

Sau khi có đủ thông tin, cập nhật file `include/config.h`:

```cpp
// Sesame 3 Configuration - THAY ĐỔI CÁC GIÁ TRỊ NÀY
#define SESAME_ADDRESS "AB:CD:EF:12:34:56"           // MAC từ scan
#define SESAME_SECRET "0102030405060708090a0b0c0d0e0f10"  // Secret từ QR
#define SESAME_PUBLIC_KEY ""                         // Để trống cho Sesame 5
```

---

## ✅ Kiểm tra thông tin

### Test kết nối với ESP32:

1. **Build và upload project**:
```bash
# Restore main.cpp
mv src/main.cpp src/scan_sesame.cpp
mv src/main.cpp.backup src/main.cpp

# Build với config mới
pio run --target upload
pio device monitor --baud 115200
```

2. **Xem logs để kiểm tra**:
```
[INFO] Scanning for Sesame devices...
[INFO] Found 1 Sesame devices:
[INFO] Connecting to Sesame 3...
[SUCCESS] Sesame 3 connected and authenticated!
```

3. **Test MQTT command**:
```bash
mosquitto_pub -h 192.168.0.200 -u cuongtq-sesame-1 -P cuongtq-sesame-1 \
  -t "sesame/command" -m '{"action":"status"}'
```

---

## 🚨 Troubleshooting

### Lỗi "No Sesame devices found":
- Đảm bảo Sesame 3 đang bật (LED nhấp nháy)
- ESP32 trong tầm Bluetooth (< 10m)
- Tắt app SESAME trên điện thoại để tránh conflict

### Lỗi "Authentication failed":
- Kiểm tra lại SESAME_SECRET
- Đảm bảo decode QR code đúng format
- Thử với Owner key thay vì Manager key

### Lỗi MAC address không đúng:
- Thử scan lại với app khác
- Kiểm tra UUID service để xác nhận đúng device
- Restart Sesame và scan lại

### Sesame không response:
- Kiểm tra pin Sesame (có thể yếu)
- Thử reset Sesame bằng cách ấn giữ nút 10 giây
- Kiểm tra firmware Sesame có update mới

---

## 📚 Tài liệu tham khảo

- [SESAME QR Reader Tool](https://sesame-qr-reader.vercel.app/)
- [libsesame3bt Documentation](https://github.com/homy-newfs8/libsesame3bt)
- [SESAME API Documentation](https://doc.candyhouse.co/)
- [nRF Connect App](https://www.nordicsemi.com/Products/Development-tools/nrf-connect-for-mobile) 