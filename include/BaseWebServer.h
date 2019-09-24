#ifndef __BASEWEBSERVER_H
#define __BASEWEBSERVER_H

#ifdef ESP32
#include <WebServer.h>
#else
#include <ESP8266WebServer.h>
#endif
#include "Customization.h"
#include "BaseConfig.h"

const char INDEX_HTML[] PROGMEM = "index.html";
const char ROOT_URI[] PROGMEM = "/";
const char SETUP_URI[] PROGMEM = "/setup";
const char CONFIG_URI[] PROGMEM = "/config";
const char SCRIPT_URI[] PROGMEM = "/script.js";
const char CSS_URI[] PROGMEM = "/styles.css";
const char RESTART_URI[] PROGMEM = "/restart";
const char SPIFFS_URI[] PROGMEM = "/spiffs";
const char FWUPDATE_URI[] PROGMEM = "/fwupdate";

class BaseWebServer {
public:
  BaseWebServer(const BaseConfig *config) : _config((BaseConfig*)config), _http(NULL) {}
  virtual ~BaseWebServer() {
    if (_http)
      delete[] _http;
  }

  virtual bool _setup();
  virtual void _loop();
  virtual void begin();

protected:
  virtual void cleanup();
  virtual void restart();

  virtual void setupHandles();

  virtual void handleNotFound();
  virtual void handleCss();
  virtual void handleScript();
  virtual void handleRoot();
  virtual void handleSetup();
  virtual void handleGetConfig();
  virtual void handleSetConfig();
  virtual void handleClearConfig();
  virtual void handleRestart();
  virtual void handleSPIFFS();
  virtual void handleFileUploaded();
  virtual void handleFileUpload();
  virtual void handleFileDelete();
  virtual void handleFwUpdate();
  virtual void handleSketchUpdated();
  virtual void handleSketchUpdate();
#ifdef USE_AUTHORIZATION
  virtual bool checkAuthorization();
#endif
  virtual bool beforeHandle() {
#ifdef USE_AUTHORIZATION
    return checkAuthorization();
#else
    return true;
#endif
  }
  virtual String getContentType(const String &fileName);
  virtual bool handleFileRead(const String &path);
  virtual String getCss();

  BaseConfig *_config;
#ifdef ESP32
  WebServer *_http;
#else
  ESP8266WebServer *_http;
#endif
};

#endif
