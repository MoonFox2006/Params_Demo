#ifndef __CAPTIVEPORTAL_H
#define __CAPTIVEPORTAL_H

#include <DNSServer.h>
#include "BaseWebServer.h"
#ifdef USE_LED
#include "Leds.h"
#endif

class CaptivePortal : public BaseWebServer {
public:
#ifdef USE_LED
  CaptivePortal(const BaseConfig *config, const Led *led) : BaseWebServer(config), _led((Led*)led), _dns(NULL) {}
#else
  CaptivePortal(const BaseConfig *config) : BaseWebServer(config), _dns(NULL) {}
#endif
  ~CaptivePortal() {
    if (_dns)
      delete[] _dns;
  }

  bool _setup();
  void _loop();

  virtual bool exec(uint16_t duration = 45);

  virtual String ssid() const;
  virtual String password() const;
  virtual uint8_t channel() const;

protected:
#ifdef USE_LED
  static const ledmode_t LED_CPWAITING = LED_4HZ;
  static const ledmode_t LED_CPPROCESSING = LED_FADEINOUT;

  void cleanup();
#endif

  void handleRoot() {
    handleSetup();
  }
  bool beforeHandle() {
    return (! isCaptivePortal());
  }

  virtual bool isCaptivePortal();

#ifdef USE_LED
  Led *_led;
#endif
  DNSServer *_dns;
};

#endif
