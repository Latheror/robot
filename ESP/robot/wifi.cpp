#include "wifi_credentials.h"
#include "wifi.h"

void setupWiFi() {
  Serial.println("\n[WiFi] Initializing...");
  Serial.printf("[WiFi] Connecting to %s\n", ssid);
  
  WiFi.mode(WIFI_STA);
  Serial.println("[WiFi] Mode set to STATION");
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    attempts++;
    if (attempts % 10 == 0) {
      Serial.printf("\n[WiFi] Still trying to connect... Attempt %d\n", attempts);
    }
  }

  Serial.println("\n[WiFi] Connection established!");
  Serial.printf("[WiFi] IP address: %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("[WiFi] Signal strength (RSSI): %d dBm\n", WiFi.RSSI());
}

void checkWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WiFi] Connection lost!");
    Serial.println("[WiFi] Attempting to reconnect...");
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
      if (attempts % 5 == 0) {
        Serial.printf("\n[WiFi] Still trying... Attempt %d/20\n", attempts);
      }
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n[WiFi] Reconnection successful!");
      Serial.printf("[WiFi] IP address: %s\n", WiFi.localIP().toString().c_str());
      Serial.printf("[WiFi] Signal strength (RSSI): %d dBm\n", WiFi.RSSI());
    } else {
      Serial.println("\n[WiFi] Reconnection failed!");
      Serial.println("[WiFi] Please check your WiFi network availability");
      
      // Print the current WiFi status
      switch (WiFi.status()) {
        case WL_IDLE_STATUS:
          Serial.println("[WiFi] Status: IDLE"); break;
        case WL_NO_SSID_AVAIL:
          Serial.println("[WiFi] Status: SSID not available"); break;
        case WL_SCAN_COMPLETED:
          Serial.println("[WiFi] Status: Scan completed"); break;
        case WL_CONNECT_FAILED:
          Serial.println("[WiFi] Status: Connection failed"); break;
        case WL_CONNECTION_LOST:
          Serial.println("[WiFi] Status: Connection lost"); break;
        case WL_DISCONNECTED:
          Serial.println("[WiFi] Status: Disconnected"); break;
        default:
          Serial.printf("[WiFi] Status: Unknown (%d)\n", WiFi.status());
      }
    }
  }
}
