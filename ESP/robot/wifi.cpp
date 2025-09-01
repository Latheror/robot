#include "wifi.h"

void setupWiFi() {
  Serial.println("\nConnexion au WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnecté au WiFi");
  Serial.print("Adresse IP: ");
  Serial.println(WiFi.localIP());
}

void checkWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connexion WiFi perdue. Tentative de reconnexion...");
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nReconnecté au WiFi");
      Serial.print("Adresse IP: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("\nÉchec de la reconnexion au WiFi");
    }
  }
}
