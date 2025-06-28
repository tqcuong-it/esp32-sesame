#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "fullhouse-2"
#define WIFI_PASSWORD "Anhcuong123"

// MQTT Configuration
#define MQTT_SERVER "192.168.0.200"
#define MQTT_PORT 1883
#define MQTT_USERNAME "cuongtq-sesame-1"
#define MQTT_PASSWORD "cuongtq-sesame-1"

// MQTT Topics
#define MQTT_TOPIC_COMMAND "sesame/command"
#define MQTT_TOPIC_STATUS "sesame/status"
#define MQTT_TOPIC_BATTERY "sesame/battery"

// Sesame Device Configuration
#define SESAME_DEVICE_NAME "セサミ4"

// Fixed Sesame Device Address (discovered and working)
#define SESAME_DEVICE_ADDRESS "************************************************"

// ⚠️ IMPORTANT: Get these keys from SESAME app QR code
// You MUST register your device via SESAME app first and scan the QR code
// to get both PUBLIC KEY and SECRET KEY

// SESAME Secret Key (32 hex characters - 16 bytes)
#define SESAME_SECRET "************************************************"

// SESAME Public Key (128 hex characters - 64 bytes) 
#define SESAME_PUBLIC_KEY "************************************************"

// Sesame Device Model
#define SESAME_MODEL_TYPE 4  // 4 = sesame_4

// Auto-test Configuration
#define AUTO_TEST_ENABLED true
#define AUTO_TEST_DELAY_MS 5000  // 5 seconds after authentication

// Debug Configuration
#define DEBUG_LEVEL 1
#define SERIAL_BAUD 115200

// RXB6 433MHz Receiver Configuration
#define RXB6_DATA_PIN 32         // GPIO pin connected to RXB6 DATA output
#define RXB6_SIGNAL_TIMEOUT 500  // Minimum time between RF signals (ms)
#define RXB6_SIGNAL_MIN_WIDTH 50 // Minimum signal width to consider valid (ms)
#define RXB6_ENABLED true        // Enable/disable RXB6 functionality

// MQTT Topics for RXB6
#define MQTT_TOPIC_RXB6 "sesame/rxb6"

#endif 