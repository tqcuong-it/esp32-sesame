#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <NimBLEDevice.h>
#include <Sesame.h>
#include <SesameClient.h>
#include "config.h"

// WiFi and MQTT clients
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Sesame client using official library
using libsesame3bt::Sesame;
using libsesame3bt::SesameClient;

SesameClient sesameClient;

// Global state variables
bool wifiConnected = false;
bool mqttConnected = false;
bool sesameConnected = false;
bool sesameAuthenticated = false;
bool autoTestCompleted = false;

SesameClient::state_t sesameState = SesameClient::state_t::idle;
SesameClient::Status lastStatus;

// RXB6 433MHz Receiver variables
volatile bool rxb6SignalReceived = false;
volatile unsigned long rxb6LastSignalTime = 0;
unsigned long rxb6LastProcessedTime = 0;

// Timing variables
unsigned long lastAutoTest = 0;
unsigned long lastConnectionAttempt = 0;
const unsigned long CONNECTION_RETRY_INTERVAL = 30000; // 30 seconds

// Function declarations
void connectToWiFi();
void connectToMQTT();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void connectToSesame();
void publishStatus();
void sendSesameCommand(String command);
void performAutoTest();

// RXB6 433MHz Receiver functions
void IRAM_ATTR rxb6InterruptHandler();
void setupRXB6();
void processRXB6Signal();
void toggleSesame();

// Sesame status callback - called when device status changes
void statusUpdate(SesameClient& client, SesameClient::Status status) {
    Serial.printf("📊 Status: lock=%u, unlock=%u, pos=%d, volt=%.2f, batt=%.1f%%\n",
                  status.in_lock(), status.in_unlock(), status.position(), 
                  status.voltage(), status.battery_pct());
    
    lastStatus = status;
    
    // Request history on status change
    client.request_history();
}

// Sesame state callback - called when connection state changes  
void stateUpdate(SesameClient& client, SesameClient::state_t state) {
    sesameState = state;
    
    String stateStr;
    switch (state) {
        case SesameClient::state_t::idle:
            stateStr = "idle";
            if (sesameConnected || sesameAuthenticated) {
                Serial.println("⚠️ Connection lost - will retry in 30 seconds");
            }
            sesameConnected = false;
            sesameAuthenticated = false;
            autoTestCompleted = false;
            break;
        case SesameClient::state_t::connected:
            stateStr = "connected";
            sesameConnected = true;
            break;
        case SesameClient::state_t::authenticating:
            stateStr = "authenticating";
            break;
        case SesameClient::state_t::active:
            stateStr = "active";
            sesameAuthenticated = true;
            lastAutoTest = millis(); // Start auto-test timer
            
            // Verify session is truly active
            if (client.is_session_active()) {
                Serial.println("🔐 Authentication successful! Session is active");
                // Request initial status
                client.request_status();
            } else {
                Serial.println("⚠️ State is active but session not confirmed");
            }
            break;
        default:
            stateStr = "unknown(" + String(static_cast<uint8_t>(state)) + ")";
            break;
    }
    
    Serial.printf("🔄 Sesame state: %s\n", stateStr.c_str());
    
    // Removed automatic MQTT publish - only publish on explicit status request
}

// History callback - called when history is received
void historyReceived(SesameClient& client, const SesameClient::History& history) {
    if (history.result != Sesame::result_code_t::success) {
        return;
    }
    
    struct tm tm;
    gmtime_r(&history.time, &tm);
    
    Serial.printf("📜 History: type=%u, %04d/%02d/%02d %02d:%02d:%02d, tag=%s\n",
                  static_cast<uint8_t>(history.type), tm.tm_year + 1900, tm.tm_mon + 1, 
                  tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, history.tag);
}

// RXB6 433MHz Receiver interrupt handler
void IRAM_ATTR rxb6InterruptHandler() {
    unsigned long currentTime = millis();
    
    // Simple debouncing - ignore signals too close together
    if (currentTime - rxb6LastSignalTime > RXB6_SIGNAL_MIN_WIDTH) {
        rxb6SignalReceived = true;
        rxb6LastSignalTime = currentTime;
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("=== ESP32 Sesame MQTT Controller ===");
    Serial.printf("📱 Device: %s\n", SESAME_DEVICE_NAME);
    Serial.printf("📍 Address: %s\n", SESAME_DEVICE_ADDRESS);
    Serial.println();
    
    // Initialize WiFi and MQTT
    connectToWiFi();
    connectToMQTT();
    
    // Initialize BLE
    Serial.println("🔵 Initializing BLE...");
    NimBLEDevice::init("ESP32_Sesame");
    NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    NimBLEDevice::setMTU(517); // Set larger MTU for better communication
    Serial.println("✅ BLE initialized");
    
    // Configure Sesame client callbacks
    sesameClient.set_state_callback(stateUpdate);
    sesameClient.set_status_callback(statusUpdate);
    sesameClient.set_history_callback(historyReceived);
    sesameClient.set_connect_timeout(15000); // 15 second timeout
    
    Serial.println("🚀 Setup completed!");
    Serial.printf("📡 WiFi: %s\n", wifiConnected ? "Connected" : "Disconnected");
    Serial.printf("📨 MQTT: %s\n", mqttConnected ? "Connected" : "Disconnected");
    Serial.println();
    
    // Setup RXB6 433MHz Receiver
    setupRXB6();
    
    // Connect to Sesame
    connectToSesame();
}

void loop() {
    // Handle WiFi reconnection
    if (!wifiConnected && WiFi.status() != WL_CONNECTED) {
        connectToWiFi();
    }
    
    // Handle MQTT reconnection
    if (wifiConnected && !mqttConnected) {
        connectToMQTT();
    }
    
    // Handle MQTT loop
    if (mqttConnected) {
        mqttClient.loop();
    }
    
    // Auto-test after authentication
    if (sesameAuthenticated && !autoTestCompleted && 
        (millis() - lastAutoTest) > AUTO_TEST_DELAY_MS) {
        performAutoTest();
    }
    
    // Process RXB6 signals
    if (rxb6SignalReceived) {
        processRXB6Signal();
    }
    
    // Removed periodic status updates - only publish when explicitly requested
    
    // Reconnect to Sesame if disconnected
    if (!sesameConnected && (millis() - lastConnectionAttempt) > CONNECTION_RETRY_INTERVAL) {
        Serial.println("🔄 Attempting to reconnect to Sesame...");
        connectToSesame();
    }
    
    delay(100);
}

void connectToWiFi() {
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        return;
    }
    
    Serial.printf("📡 Connecting to WiFi: %s\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        Serial.printf("\n✅ WiFi connected: %s\n", WiFi.localIP().toString().c_str());
    } else {
        wifiConnected = false;
        Serial.println("\n❌ WiFi connection failed");
    }
}

void connectToMQTT() {
    if (!wifiConnected) return;
    
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
    
    Serial.printf("📨 Connecting to MQTT: %s:%d\n", MQTT_SERVER, MQTT_PORT);
    
    if (mqttClient.connect("ESP32_Sesame", MQTT_USERNAME, MQTT_PASSWORD)) {
        mqttConnected = true;
        Serial.println("✅ MQTT connected");
        
        // Subscribe to command topic
        mqttClient.subscribe(MQTT_TOPIC_COMMAND);
        Serial.printf("📥 Subscribed to: %s\n", MQTT_TOPIC_COMMAND);
        
        // Removed startup message - only publish when explicitly requested
    } else {
        mqttConnected = false;
        Serial.printf("❌ MQTT connection failed, rc=%d\n", mqttClient.state());
    }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String message = "";
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    
    Serial.printf("📥 MQTT [%s]: %s\n", topic, message.c_str());
    
    if (String(topic) == MQTT_TOPIC_COMMAND) {
        // Parse JSON command
        DynamicJsonDocument doc(256);
        DeserializationError error = deserializeJson(doc, message);
        
        if (error) {
            Serial.printf("❌ Failed to parse JSON: %s\n", error.c_str());
            return;
        }
        
        String action = doc["action"].as<String>();
        sendSesameCommand(action);
    }
}

void connectToSesame() {
    lastConnectionAttempt = millis();
    
    Serial.printf("🔗 Connecting to Sesame: %s\n", SESAME_DEVICE_ADDRESS);
    
    // Small delay before connection attempt
    delay(1000);
    
    // Setup client with fixed address
    NimBLEAddress deviceAddress(SESAME_DEVICE_ADDRESS, BLE_ADDR_RANDOM);
    Sesame::model_t model = Sesame::model_t::sesame_4;
    
    if (!sesameClient.begin(deviceAddress, model)) {
        Serial.println("❌ Failed to begin Sesame client");
        return;
    }
    
    // Set keys
    String secret_lower = String(SESAME_SECRET);
    String public_lower = String(SESAME_PUBLIC_KEY);
    secret_lower.toLowerCase();
    public_lower.toLowerCase();
    
    if (!sesameClient.set_keys(public_lower.c_str(), secret_lower.c_str())) {
        Serial.println("❌ Failed to set keys");
        return;
    }
    
    Serial.println("🔑 Keys set, attempting connection...");
    
    // Connect using official method with more retries
    if (!sesameClient.connect(5)) {  // 5 retries instead of 3
        Serial.println("❌ Failed to connect to Sesame");
        Serial.println("💡 Please ensure:");
        Serial.println("   - Sesame app is completely closed");
        Serial.println("   - Device is not connected to other services");
        Serial.println("   - ESP32 is close to Sesame device");
        return;
    }
    
    Serial.println("✅ Connection initiated successfully");
}

void sendSesameCommand(String command) {
    if (!sesameAuthenticated) {
        Serial.println("❌ Sesame not authenticated");
        return;
    }
    
    Serial.printf("🔧 Sending command: %s\n", command.c_str());
    
    if (command == "unlock") {
        sesameClient.unlock("ESP32 unlock");
    } else if (command == "lock") {
        sesameClient.lock("ESP32 lock");
    } else if (command == "status") {
        sesameClient.request_status();
        // Wait a moment for status to update, then publish
        delay(500);
        publishStatus();
        Serial.println("📤 Status published in response to status request");
    } else {
        Serial.printf("❌ Unknown command: %s\n", command.c_str());
    }
}

void performAutoTest() {
    if (!sesameAuthenticated) return;
    
    Serial.println("⚡ Starting auto-test sequence...");
    Serial.println("🔓 Auto-test: UNLOCK");
    
    sesameClient.unlock("Auto-test unlock");
    autoTestCompleted = true;
    
    Serial.println("⏰ Auto-test: LOCK will be sent in 10 seconds via MQTT command");
}

void publishStatus() {
    if (!mqttConnected) return;
    
    DynamicJsonDocument doc(512);
    doc["device"] = SESAME_DEVICE_NAME;
    doc["address"] = SESAME_DEVICE_ADDRESS;
    doc["wifi_connected"] = wifiConnected;
    doc["mqtt_connected"] = mqttConnected;
    doc["sesame_connected"] = sesameConnected;
    doc["sesame_authenticated"] = sesameAuthenticated;
    
    if (sesameAuthenticated) {
        doc["battery_pct"] = lastStatus.battery_pct();
        doc["voltage"] = lastStatus.voltage();
        doc["position"] = lastStatus.position();
        doc["locked"] = lastStatus.in_lock();
        doc["unlocked"] = lastStatus.in_unlock();
    }
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    mqttClient.publish(MQTT_TOPIC_STATUS, jsonString.c_str());
}

// RXB6 433MHz Receiver setup
void setupRXB6() {
    if (!RXB6_ENABLED) {
        Serial.println("📡 RXB6 disabled in configuration");
        return;
    }
    
    Serial.printf("📡 Setting up RXB6 433MHz receiver on GPIO %d\n", RXB6_DATA_PIN);
    
    // Configure pin as input with pullup
    pinMode(RXB6_DATA_PIN, INPUT_PULLUP);
    
    // Attach interrupt on falling edge (signal received)
    attachInterrupt(digitalPinToInterrupt(RXB6_DATA_PIN), rxb6InterruptHandler, FALLING);
    
    Serial.println("✅ RXB6 setup completed");
    Serial.println("📻 Ready to receive 433MHz signals");
}

// Process received RXB6 signal
void processRXB6Signal() {
    unsigned long currentTime = millis();
    
    // Check timeout to prevent spam
    if (currentTime - rxb6LastProcessedTime < RXB6_SIGNAL_TIMEOUT) {
        rxb6SignalReceived = false;
        return;
    }
    
    rxb6SignalReceived = false;
    rxb6LastProcessedTime = currentTime;
    
    Serial.println("📻 RXB6: 433MHz signal received!");
    
    // Publish MQTT notification
    if (mqttConnected) {
        DynamicJsonDocument doc(256);
        doc["signal"] = "received";
        doc["timestamp"] = currentTime;
        doc["pin"] = RXB6_DATA_PIN;
        
        String jsonString;
        serializeJson(doc, jsonString);
        
        mqttClient.publish(MQTT_TOPIC_RXB6, jsonString.c_str());
        Serial.printf("📤 RXB6 MQTT: %s\n", jsonString.c_str());
    }
    
    // Toggle Sesame if authenticated
    if (sesameAuthenticated) {
        toggleSesame();
    } else {
        Serial.println("⚠️ Sesame not authenticated - ignoring RXB6 signal");
    }
}

// Toggle Sesame lock/unlock based on current state
void toggleSesame() {
    if (!sesameAuthenticated) {
        Serial.println("❌ Cannot toggle - Sesame not authenticated");
        return;
    }
    
    // Determine current state and toggle
    bool isLocked = lastStatus.in_lock();
    bool isUnlocked = lastStatus.in_unlock();
    
    String action;
    String reason = "RXB6 433MHz";
    
    if (isLocked) {
        // Currently locked - unlock it
        action = "unlock";
        Serial.println("🔓 RXB6 trigger: UNLOCKING Sesame");
        sesameClient.unlock(reason.c_str());
    } else if (isUnlocked) {
        // Currently unlocked - lock it  
        action = "lock";
        Serial.println("🔒 RXB6 trigger: LOCKING Sesame");
        sesameClient.lock(reason.c_str());
    } else {
        // Unknown state - default to unlock
        action = "unlock";
        Serial.println("❓ RXB6 trigger: Unknown state, defaulting to UNLOCK");
        sesameClient.unlock(reason.c_str());
    }
    
    // Publish MQTT action notification
    if (mqttConnected) {
        DynamicJsonDocument doc(256);
        doc["action"] = action;
        doc["trigger"] = "rxb6";
        doc["reason"] = reason;
        doc["timestamp"] = millis();
        
        String jsonString;
        serializeJson(doc, jsonString);
        
        mqttClient.publish(MQTT_TOPIC_STATUS, jsonString.c_str());
        Serial.printf("📤 Sesame %s triggered by RXB6\n", action.c_str());
    }
} 