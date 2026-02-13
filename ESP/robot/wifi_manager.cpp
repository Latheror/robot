#include "wifi_manager.h"
#include "settings.h"
#include <Arduino.h>

bool WiFiManager::connect() {
    Serial.println("[WiFi] Initializing connection...");
    
    // Configure WiFi in station mode
    WiFi.mode(WIFI_STA);
    WiFi.begin(NetworkConfig::WIFI_SSID, NetworkConfig::WIFI_PASSWORD);
    
    // Wait for connection with timeout
    unsigned long startAttemptTime = millis();
    
    while (WiFi.status() != WL_CONNECTED && 
           millis() - startAttemptTime < NetworkConfig::WIFI_TIMEOUT_MS) {
        delay(NetworkConfig::WIFI_RETRY_DELAY);
        
        // Log progress every second
        if ((millis() - startAttemptTime) % NetworkConfig::WIFI_LOG_INTERVAL_MS == 0) {
            Serial.printf("[WiFi] Connecting... (%d ms elapsed)\n", 
                         (int)(millis() - startAttemptTime));
        }
    }
    
    // Check connection result
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("[WiFi] Connected to %s\n", NetworkConfig::WIFI_SSID);
        Serial.printf("[WiFi] IP: %s, RSSI: %d dBm\n", 
                     WiFi.localIP().toString().c_str(), 
                     WiFi.RSSI());
        return true;
    }
    
    Serial.printf("[WiFi] Connection failed: %s\n", 
                  getStatusString(WiFi.status()).c_str());
    return false;
}

bool WiFiManager::checkConnection() {
    if (isConnected()) {
        return true;
    }

    Serial.println("[WiFi] Connection lost, attempting to reconnect...");
    return connect();
}

/**
 * @brief Get string representation of WiFi status.
 * @param status The WiFi status.
 * @return Status string.
 */
String WiFiManager::getStatusString(wl_status_t status) {
    switch (status) {
        case WL_IDLE_STATUS:     return "Idle";
        case WL_NO_SSID_AVAIL:   return "SSID not available";
        case WL_SCAN_COMPLETED:  return "Scan completed";
        case WL_CONNECT_FAILED:  return "Connection failed";
        case WL_CONNECTION_LOST: return "Connection lost";
        case WL_DISCONNECTED:    return "Disconnected";
        default:                 return "Unknown status";
    }
}
