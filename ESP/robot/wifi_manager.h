#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>

/**
 * @class WiFiManager
 * @brief Manages WiFi connection for the ESP32.
 */
class WiFiManager {
public:
    /**
     * @brief Initialize and connect to WiFi network.
     * @return true if connected, false otherwise.
     */
    static bool connect();

    /**
     * @brief Check and maintain WiFi connection.
     * @return true if connected, false otherwise.
     */
    static bool checkConnection();

    /**
     * @brief Get current connection status.
     * @return true if connected, false otherwise.
     */
    static bool isConnected() { return WiFi.status() == WL_CONNECTED; }

    /**
     * @brief Get current IP address.
     * @return The local IP address.
     */
    static IPAddress getLocalIP() { return WiFi.localIP(); }

    /**
     * @brief Get current signal strength.
     * @return RSSI value.
     */
    static int getSignalStrength() { return WiFi.RSSI(); }

private:
    static const int MAX_CONNECTION_ATTEMPTS = 20; ///< Max connection attempts
    static const int CONNECTION_TIMEOUT = 500;     ///< Connection timeout in ms

    /**
     * @brief Get string representation of WiFi status.
     * @param status The WiFi status.
     * @return Status string.
     */
    static String getStatusString(wl_status_t status);
};

#endif // WIFI_MANAGER_H
