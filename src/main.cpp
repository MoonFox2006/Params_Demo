#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "BaseConfig.h"
#include "Leds.h"
#include "BaseWebServer.h"
#include "CaptivePortal.h"

const char WIFI_SSID_PARAM[] PROGMEM = "wifi_ssid";
const char WIFI_SSID_DESCR[] PROGMEM = "WiFi SSID";
const char WIFI_PSWD_PARAM[] PROGMEM = "wifi_pswd";
const char WIFI_PSWD_DESCR[] PROGMEM = "WiFi password";
const char NTP_SERVER_PARAM[] PROGMEM = "ntp_server";
const char NTP_SERVER_DESCR[] PROGMEM = "NTP server";
const char NTP_SERVER_DEF[] PROGMEM = "pool.ntp.org";
const char NTP_TZ_PARAM[] PROGMEM = "ntp_tz";
const char NTP_TZ_DESCR[] PROGMEM = "NTP time zone";
const char NTP_UPDATE_PARAM[] PROGMEM = "ntp_update";
const char NTP_UPDATE_DESCR[] PROGMEM = "NTP auto update";

const param_t PARAMS[] PROGMEM = {
  PARAM_STR(WIFI_SSID_PARAM, WIFI_SSID_DESCR, 32, NULL),
  PARAM_PSWD(WIFI_PSWD_PARAM, WIFI_PSWD_DESCR, 32, NULL),
  PARAM_STR(NTP_SERVER_PARAM, NTP_SERVER_DESCR, 32, NTP_SERVER_DEF),
  PARAM_I8(NTP_TZ_PARAM, NTP_TZ_DESCR, 3),
  PARAM_BOOL(NTP_UPDATE_PARAM, NULL, false)
};

class Config : public BaseConfig {
public:
  Config() : BaseConfig(PARAMS, 5) {}

  void *getParamPtr(uint8_t index);

  struct __packed {
    char _wifi_ssid[32];
    char _wifi_pswd[32];
    char _ntp_server[32];
    int8_t _ntp_tz;
    bool _ntp_update;
  };
};

void *Config::getParamPtr(uint8_t index) {
  if (index == 0)
    return _wifi_ssid;
  else if (index == 1)
    return _wifi_pswd;
  else if (index == 2)
    return _ntp_server;
  else if (index == 3)
    return &_ntp_tz;
  else if (index == 4)
    return &_ntp_update;

  return NULL;
}

Config *config;
Led *led;
BaseWebServer *http;

bool wifiConnect() {
  const uint32_t WIFI_TIMEOUT = 60000;

  static uint32_t lastTry = 0;

  if (WiFi.isConnected())
    return true;

  if ((! lastTry) || (millis() - lastTry > WIFI_TIMEOUT)) {
    WiFi.begin(config->_wifi_ssid, config->_wifi_pswd);
    Serial.print(F("Connecting to WiFi \""));
    Serial.print(config->_wifi_ssid);
    Serial.print('"');
    lastTry = millis();
    led->setMode(LED_2HZ);
    while (! WiFi.isConnected()) {
      if (millis() - lastTry > WIFI_TIMEOUT)
        break;
      Serial.print('.');
      led->delay(500);
    }
    if (WiFi.isConnected()) {
      Serial.print(F(" OK (IP "));
      Serial.print(WiFi.localIP());
      Serial.println(')');
      http->begin();
      lastTry = 0;
      led->setMode(LED_FADEINOUT);

      return true;
    } else {
      Serial.println(F(" FAIL!"));
      lastTry = millis();
      led->setMode(LED_OFF);

//      return false;
    }
  }

  return false;
}

void setup() {
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
  Serial.println();

  if (! initSPIFFS()) {
    Serial.println(F("SPIFFS init fail!"));
    Serial.flush();
    ESP.deepSleep(0);
  }
  config = new Config();
  if (! config->load()) {
    config->clear();
    Serial.println(F("Use default config"));
  }
  led = new Led(LED_BUILTIN, LOW);

  {
    CaptivePortal cp(config, led);

    cp.exec(60);
  }

  WiFi.mode(WIFI_STA);

  http = new BaseWebServer(config);
  http->_setup();
}

void loop() {
  if (! WiFi.isConnected()) {
    wifiConnect();
  }
  if (WiFi.isConnected())
    http->_loop();
  led->delay(1);
}
