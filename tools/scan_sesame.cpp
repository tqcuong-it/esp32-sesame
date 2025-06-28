/*
 * ESP32 Simple BLE Scanner for Sesame Devices
 * Tìm kiếm tất cả thiết bị BLE có thể là Sesame
 */

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEAddress.h>

// Cấu hình scanner
const int SCAN_TIME = 10; // seconds
const int MAX_DEVICES = 20;
unsigned long lastScan = 0;
const unsigned long SCAN_INTERVAL = 15000; // 15 seconds

// Lưu trữ devices tìm thấy
struct BLEDeviceInfo {
    String address;
    String name;
    int rssi;
    bool hasSesameUUID;
    bool hasManufacturerData;
    String manufacturerData;
    unsigned long lastSeen;
};

BLEDeviceInfo foundDevices[MAX_DEVICES];
int deviceCount = 0;
int scanNumber = 0;

// UUID của Sesame (một số UUID thường thấy)
const char* SESAME_SERVICE_UUIDS[] = {
    "16860000-a5ae-9856-b6d6-a87a2e2cb400",  // Sesame service UUID
    "0000fd81-0000-1000-8000-00805f9b34fb",  // Sesame service UUID alternative
    "0000180f-0000-1000-8000-00805f9b34fb"   // Battery service
};

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        String address = advertisedDevice.getAddress().toString().c_str();
        String name = advertisedDevice.getName().c_str();
        int rssi = advertisedDevice.getRSSI();
        bool hasSesameUUID = false;
        bool hasManufData = false;
        String manufacData = "";
        
        // Check for Sesame service UUIDs
        if (advertisedDevice.haveServiceUUID()) {
            for (int i = 0; i < 3; i++) {
                if (advertisedDevice.isAdvertisingService(BLEUUID(SESAME_SERVICE_UUIDS[i]))) {
                    hasSesameUUID = true;
                    break;
                }
            }
        }
        
        // Check manufacturer data
        if (advertisedDevice.haveManufacturerData()) {
            hasManufData = true;
            std::string data = advertisedDevice.getManufacturerData();
            for (size_t i = 0; i < data.length(); i++) {
                if (i > 0) manufacData += ":";
                manufacData += String(data[i], HEX);
            }
        }
        
        // Lọc thiết bị có thể là Sesame
        bool couldBeSesame = false;
        
        // Điều kiện 1: Có Sesame UUID
        if (hasSesameUUID) {
            couldBeSesame = true;
        }
        
        // Điều kiện 2: Tên chứa "sesame", "candy" hoặc MAC có pattern phù hợp
        String nameLower = name;
        nameLower.toLowerCase();
        if (nameLower.indexOf("sesame") >= 0 || 
            nameLower.indexOf("candy") >= 0 ||
            nameLower.indexOf("ss3") >= 0) {
            couldBeSesame = true;
        }
        
        // Điều kiện 3: Thiết bị BLE không tên nhưng có manufacturer data
        if (name.length() == 0 && hasManufData && rssi > -80) {
            couldBeSesame = true;
        }
        
        // Chỉ lưu thiết bị có khả năng là Sesame
        if (couldBeSesame) {
            addOrUpdateDevice(address, name, rssi, hasSesameUUID, hasManufData, manufacData);
        }
        
        // In thông tin realtime cho thiết bị tiềm năng
        if (couldBeSesame) {
            Serial.print("🔍 Found: ");
            Serial.print(address);
            if (name.length() > 0) {
                Serial.print(" (");
                Serial.print(name);
                Serial.print(")");
            }
            Serial.print(" RSSI: ");
            Serial.print(rssi);
            if (hasSesameUUID) Serial.print(" [SESAME UUID]");
            Serial.println();
        }
    }
};

void addOrUpdateDevice(String address, String name, int rssi, bool hasUUID, bool hasManuf, String manufacData) {
    // Tìm device đã tồn tại
    for (int i = 0; i < deviceCount; i++) {
        if (foundDevices[i].address.equalsIgnoreCase(address)) {
            // Cập nhật thông tin
            foundDevices[i].rssi = rssi;
            foundDevices[i].lastSeen = millis();
            if (name.length() > 0 && foundDevices[i].name.length() == 0) {
                foundDevices[i].name = name;
            }
            return;
        }
    }
    
    // Thêm device mới
    if (deviceCount < MAX_DEVICES) {
        foundDevices[deviceCount].address = address;
        foundDevices[deviceCount].name = name;
        foundDevices[deviceCount].rssi = rssi;
        foundDevices[deviceCount].hasSesameUUID = hasUUID;
        foundDevices[deviceCount].hasManufacturerData = hasManuf;
        foundDevices[deviceCount].manufacturerData = manufacData;
        foundDevices[deviceCount].lastSeen = millis();
        deviceCount++;
    }
}

void printResults() {
    Serial.println("\n" + String(60, '='));
    Serial.println("📡 BLE DEVICE SCANNER RESULTS");
    Serial.println(String(60, '='));
    
    if (deviceCount == 0) {
        Serial.println("❌ No potential Sesame devices found!");
        Serial.println("\n🔍 Troubleshooting:");
        Serial.println("1. Make sure Sesame is powered on (LED blinking)");
        Serial.println("2. Move ESP32 closer to Sesame (< 5m)");
        Serial.println("3. Close SESAME app on phone");
        Serial.println("4. Try pressing button on Sesame to wake it up");
        return;
    }
    
    Serial.printf("✅ Found %d potential Sesame device(s):\n\n", deviceCount);
    
    for (int i = 0; i < deviceCount; i++) {
        Serial.println("📱 Device #" + String(i + 1));
        Serial.println("   📍 MAC Address: " + foundDevices[i].address);
        
        if (foundDevices[i].name.length() > 0) {
            Serial.println("   🏷️  Name: " + foundDevices[i].name);
        } else {
            Serial.println("   🏷️  Name: (No name)");
        }
        
        Serial.println("   📶 RSSI: " + String(foundDevices[i].rssi) + " dBm");
        
        if (foundDevices[i].hasSesameUUID) {
            Serial.println("   ✅ Has Sesame UUID: YES");
        } else {
            Serial.println("   ⚠️  Has Sesame UUID: NO");
        }
        
        if (foundDevices[i].hasManufacturerData) {
            Serial.println("   📊 Manufacturer Data: " + foundDevices[i].manufacturerData);
        }
        
        // Hiển thị cấu hình
        Serial.println("   ⚙️  Config for this device:");
        Serial.println("       #define SESAME_ADDRESS \"" + foundDevices[i].address + "\"");
        
        Serial.println();
    }
    
    Serial.println("🎯 NEXT STEPS:");
    Serial.println("1. Try each MAC address above in your config");
    Serial.println("2. Get SECRET key from SESAME app QR code:");
    Serial.println("   https://sesame-qr-reader.vercel.app/");
    Serial.println("3. Update include/config.h with MAC and SECRET");
    Serial.println("4. Restore main.cpp and test connection");
    Serial.println();
    Serial.println("📝 Note: Device with 'Has Sesame UUID: YES' is most likely your Sesame");
}

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    
    Serial.println("🚀 ESP32 BLE SCANNER FOR SESAME DEVICES");
    Serial.println("======================================");
    Serial.println("🔍 Scanning for BLE devices that could be Sesame...");
    Serial.println("📱 Make sure to close SESAME app on your phone");
    Serial.println("⏱️  Each scan takes 10 seconds, will run multiple scans");
    Serial.println();
    
    BLEDevice::init("ESP32-Sesame-Scanner");
}

void loop() {
    if (millis() - lastScan > SCAN_INTERVAL) {
        lastScan = millis();
        scanNumber++;
        
        Serial.println("🔄 Scan #" + String(scanNumber) + " starting...");
        
        BLEScan* pBLEScan = BLEDevice::getScan();
        pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
        pBLEScan->setActiveScan(true);
        pBLEScan->setInterval(100);
        pBLEScan->setWindow(99);
        
        BLEScanResults foundDevicesThisScan = pBLEScan->start(SCAN_TIME, false);
        
        Serial.printf("   Scan completed. Found %d devices in this scan\n", foundDevicesThisScan.getCount());
        Serial.printf("   Total potential Sesame devices: %d\n", deviceCount);
        
        pBLEScan->clearResults();
        
        // Hiển thị kết quả sau scan thứ 2 hoặc nếu tìm thấy devices
        if (scanNumber >= 2 || deviceCount > 0) {
            printResults();
            
            if (deviceCount > 0) {
                Serial.println("⏹️  Scanner will continue running...");
                Serial.println("   Press RESET to restart or upload main project");
            }
        }
        
        Serial.println("⏳ Waiting " + String(SCAN_INTERVAL/1000) + " seconds before next scan...\n");
    }
    
    delay(1000);
} 