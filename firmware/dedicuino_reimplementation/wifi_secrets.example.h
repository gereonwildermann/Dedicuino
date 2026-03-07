#pragma once

// Copy this file to wifi_secrets.h and set your real credentials.
// wifi_secrets.h is ignored by git.

#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// Optional OTA settings (ESP32)
// Hostname shown in Arduino IDE network ports.
#define WIFI_OTA_HOSTNAME "dedicuino"

// Leave empty to disable OTA authentication, or set a password.
#define WIFI_OTA_PASSWORD ""
