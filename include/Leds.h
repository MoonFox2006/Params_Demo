#ifndef __LEDS_H
#define __LEDS_H

#define ONE_LED

#include <inttypes.h>
#ifndef ONE_LED
#include <List.h>
#endif

enum ledmode_t : uint8_t { LED_OFF, LED_ON, LED_1HZ, LED_2HZ, LED_4HZ, LED_FADEIN, LED_FADEOUT, LED_FADEINOUT };

struct __packed _led_t {
  uint8_t pin : 4;
  bool level : 1;
  ledmode_t mode : 3;
};

#ifdef ONE_LED
class Led {
public:
  Led(uint8_t pin, bool level = false);

  ledmode_t getMode() const {
    return _item.mode;
  }
  void setMode(ledmode_t mode) {
    _item.mode = mode;
    update(true);
  }
  void update(bool force = false);
#else
class Leds : public List<_led_t, 10> {
public:
  Leds() : List<_led_t, 10>() {};

  uint8_t add(uint8_t pin, bool level, ledmode_t mode);
  ledmode_t getMode(uint8_t index) const;
  void setMode(uint8_t index, ledmode_t mode);
  void update(uint8_t index = ERR_INDEX, bool force = false);
#endif
  void delay(uint32_t ms);

protected:
  static const uint8_t GPIO16_RENUM = 6; // Renum GPIO16 to unused GPIO6

  static const uint32_t BLINK_TIME = 25; // 25 ms.

  static uint8_t pinToGpio(uint8_t pin) {
    return (pin == GPIO16_RENUM) ? 16 : pin;
  }

#ifndef ONE_LED
  bool match(uint8_t index, const void *t);

  void on(uint8_t index);
  void off(uint8_t index);
#else
  inline void off();
  inline void on();

  _led_t _item;
#endif
};

#endif
