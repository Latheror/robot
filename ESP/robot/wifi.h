#ifndef WIFI_H
#define WIFI_H

#include <ESP8266WiFi.h>
#include "wifi_credentials.h"  // Include credentials from separate file

// Function declarations
void setupWiFi();
void checkWiFiConnection();

#endif // WIFI_H
