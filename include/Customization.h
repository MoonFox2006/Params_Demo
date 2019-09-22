#ifndef __CUSTOMIZATION_H
#define __CUSTOMIZATION_H

#define USE_SERIAL // Use UART for output
#define USE_LED // Use led for visualization
//#define USE_AUTHORIZATION // Use web page basic authorization

#ifdef USE_AUTHORIZATION
#define AUTH_USER "ESP" // User name for basic authorization
#define AUTH_PSWD "12345678" // Password for basic authorization
#endif

//#define CP_SSID "ESP" // Non-automatic Captive Portal AP name
//#define CP_PSWD "1029384756" // Non-automatic Captive Portal AP password

#define CP_PREFIX "ESP_" // Captive Portal AP name prefix
#define CP_SALT "12" // Captive Portal AP password suffix

#define MAX_WIFI_CHANNEL 13 // Max WiFi channel for AP (11 for North America, 13 for Europe)

#endif
