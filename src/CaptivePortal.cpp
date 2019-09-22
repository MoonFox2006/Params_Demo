#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include "CaptivePortal.h"
#include "StrUtils.h"
#include "HtmlHelper.h"

static uint8_t wifiFindFreeChannel() {
  int32_t levels[MAX_WIFI_CHANNEL];
  int8_t nets, i;

  memset(levels, 0, sizeof(levels));
  WiFi.disconnect();
  nets = WiFi.scanNetworks(false, true);
  for (i = 0; i < nets; ++i) {
    uint8_t channel = WiFi.channel(i);
    int32_t rssi = WiFi.RSSI(i);

    if ((! levels[channel - 1]) || (levels[channel - 1] < rssi))
      levels[channel - 1] = rssi;
  }
  WiFi.scanDelete();

  nets = 0; // First channel
  int32_t minlevel = levels[nets] + levels[nets + 1] / 2;

  if (minlevel) {
    for (i = 1; i < MAX_WIFI_CHANNEL; ++i) {
      int32_t level = levels[i] + levels[i - 1] / 2;

      if (i < MAX_WIFI_CHANNEL - 1)
        level += levels[i + 1] / 2;
      if (! level) { // Free channel
        nets = i;
        break;
      } else if (level < minlevel) {
        minlevel = level;
        nets = i;
      }
    }
  }

  return (nets + 1);
}

bool CaptivePortal::_setup() {
  _dns = new DNSServer();
  if (! _dns) {
#ifdef USE_SERIAL
    Serial.println(F("DNS server init error!"));
#endif

    return false;
  }

  _dns->setErrorReplyCode(DNSReplyCode::NoError);
  if (! _dns->start(53, F("*"), WiFi.softAPIP())) {
#ifdef USE_SERIAL
    Serial.println(F("DNS server start error!"));
#endif

    return false;
  }

  if (! BaseWebServer::_setup()) {
    delete _dns;
    _dns = NULL;
#ifdef USE_SERIAL
    Serial.println(F("Web server init error!"));
#endif

    return false;
  }

  return true;
}

void CaptivePortal::_loop() {
  if (_dns)
    _dns->processNextRequest();
  BaseWebServer::_loop();
}

bool CaptivePortal::exec(uint16_t duration) {
  {
    String _ssid = ssid();
    String _pswd = password();
    uint8_t _channel = channel();

    WiFi.mode(WIFI_AP);
#ifdef USE_SERIAL
    Serial.print(F("AP \""));
    Serial.print(_ssid);
    Serial.print(F("\" with password \""));
    Serial.print(_pswd);
    Serial.print(F("\" on channel "));
    Serial.print(_channel);
    Serial.print(F(" setup "));
#endif
    if (! WiFi.softAP(_ssid.c_str(), _pswd.c_str(), _channel)) {
#ifdef USE_SERIAL
      Serial.println(F("FAIL!"));
#endif

      return false;
    }
  }
#ifdef USE_SERIAL
  Serial.print(F("successfully (IP: "));
  Serial.print(WiFi.softAPIP());
  if (duration) {
    Serial.print(F(") for "));
    Serial.print(duration);
    Serial.println(F(" seconds"));
  } else {
    Serial.println(')');
  }
#endif

  bool success = _setup();

  if (success) {
#ifdef USE_SERIAL
    Serial.print(F("Visit to http://"));
    Serial.print(WiFi.softAPIP());
    Serial.println(FPSTR(ROOT_URI));
#endif

    uint32_t start = millis();

    while ((! duration) || (millis() - start < duration * 1000)) {
      _loop();
      if (WiFi.softAPgetStationNum()) {
        start = millis();
#ifdef USE_LED
        _led->setMode(LED_CPPROCESSING);
      } else {
        _led->setMode(LED_CPWAITING);
      }
      _led->delay(1);
#else
      }
      delay(1);
#endif
    }
#ifdef USE_LED
    _led->setMode(LED_OFF);
#endif
    _http->close();
    delete _http;
    _http = NULL;
    _dns->stop();
    delete _dns;
    _dns = NULL;
  }

  WiFi.softAPdisconnect(true);
#ifdef USE_SERIAL
  Serial.println(F("Access Point closed"));
#endif

  return success;
}

String CaptivePortal::ssid() const {
#ifdef CP_SSID
  return String(F(CP_SSID));
#else
  String result;
  uint32_t id;
  char hex[3];

#ifdef ESP32
  id = ESP.getEfuseMac() >> 16;
#else
  id = ESP.getChipId();
#endif
  result = FPSTR(CP_PREFIX);
  for (uint8_t i = 0; i < 4; ++i) {
#ifdef ESP32
    result += byteToHex(hex, id >> (8 * i));
#else
    result += byteToHex(hex, id >> (8 * (3 - i)));
#endif
  }

  return result;
#endif
}

String CaptivePortal::password() const {
#ifdef CP_PSWD
  return String(F(CP_PSWD));
#else
  String result;
  uint32_t id;
  char hex[3];

#ifdef ESP32
  id = ESP.getEfuseMac() >> 16;
#else
  id = ESP.getChipId();
#endif
  for (uint8_t i = 0; i < 4; ++i) {
#ifdef ESP32
    result += byteToHex(hex, id >> (8 * i));
#else
    result += byteToHex(hex, id >> (8 * (3 - i)));
#endif
  }
  result += FPSTR(CP_SALT); // Password suffix

  return result;
#endif
}

uint8_t CaptivePortal::channel() const {
  return wifiFindFreeChannel();
}

#ifdef USE_LED
void CaptivePortal::cleanup() {
  _led->setMode(LED_OFF);
  BaseWebServer::cleanup();
}
#endif

bool CaptivePortal::isCaptivePortal() {
  if (! _http->hostHeader().equals(WiFi.softAPIP().toString())) {
    _http->sendHeader(F("Location"), String(F("http://")) + WiFi.softAPIP().toString(), true);
    _http->send_P(302, TEXT_PLAIN, NULL, 0);

    return true;
  }

  return false;
}
