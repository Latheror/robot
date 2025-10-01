#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>

class WiFiManager {
public:
    // Initialize and connect to WiFi network
    static bool connect();
    
    // Check and maintain WiFi connection
    static bool checkConnection();
    
    // Get current connection status
    static bool isConnected() { return WiFi.status() == WL_CONNECTED; }
    
    // Get current IP address
    static IPAddress getLocalIP() { return WiFi.localIP(); }
    
    // Get current signal strength
    static int getSignalStrength() { return WiFi.RSSI(); }

private:
    static const int MAX_CONNECTION_ATTEMPTS = 20;
    static const int CONNECTION_TIMEOUT = 500; // ms
    static String getStatusString(wl_status_t status);
};

#endif // WIFI_MANAGER_H
