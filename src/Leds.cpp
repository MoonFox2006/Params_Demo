#include <Arduino.h>
#include "Leds.h"

#ifdef ONE_LED
Led::Led(uint8_t pin, bool level) {
  _item.pin = (pin == 16) ? GPIO16_RENUM : pin;
  _item.level = level;
  _item.mode = LED_OFF;
  pinMode(pin, OUTPUT);
  off();
}

void Led::update(bool force) {
  if (force || (_item.mode > LED_ON)) {
    if (_item.mode == LED_OFF) {
      off();
    } else if (_item.mode == LED_ON) {
      on();
    } else {
      uint16_t subsec;

      if (_item.mode == LED_FADEINOUT)
        subsec = millis() % 2000;
      else
        subsec = millis() % 1000;
      if (_item.mode == LED_1HZ) {
        if (subsec < BLINK_TIME)
          on();
        else
          off();
      } else if (_item.mode == LED_2HZ) {
        if (subsec < BLINK_TIME)
          on();
        else if (subsec < 500)
          off();
        else if (subsec < 500 + BLINK_TIME)
          on();
        else
          off();
      } else if (_item.mode == LED_4HZ) {
        if (subsec < BLINK_TIME)
          on();
        else if (subsec < 250)
          off();
        else if (subsec < 250 + BLINK_TIME)
          on();
        else if (subsec < 500)
          off();
        else if (subsec < 500 + BLINK_TIME)
          on();
        else if (subsec < 750)
          off();
        else if (subsec < 750 + BLINK_TIME)
          on();
        else
          off();
      } else if (_item.mode == LED_FADEIN) {
        if (_item.level)
          analogWrite(pinToGpio(_item.pin), map(subsec, 0, 999, 0, 1023));
        else
          analogWrite(pinToGpio(_item.pin), map(subsec, 0, 999, 1023, 0));
      } else if (_item.mode == LED_FADEOUT) {
        if (_item.level)
          analogWrite(pinToGpio(_item.pin), map(subsec, 0, 999, 1023, 0));
        else
          analogWrite(pinToGpio(_item.pin), map(subsec, 0, 999, 0, 1023));
      } else if (_item.mode == LED_FADEINOUT) {
        if (subsec < 1000) {
          if (_item.level)
            analogWrite(pinToGpio(_item.pin), map(subsec, 0, 999, 0, 1023));
          else
            analogWrite(pinToGpio(_item.pin), map(subsec, 0, 999, 1023, 0));
        } else {
          if (_item.level)
            analogWrite((_item.pin), map(subsec, 1000, 1999, 1023, 0));
          else
            analogWrite((_item.pin), map(subsec, 1000, 1999, 0, 1023));
        }
      }
    }
  }
}

void Led::delay(uint32_t ms) {
  if (_item.mode < LED_1HZ)
    ::delay(ms);
  else {
    while (ms--) {
      update();
      ::delay(1);
    }
  }
}

inline void Led::off() {
  digitalWrite(pinToGpio(_item.pin), ! _item.level);
}

inline void Led::on() {
  digitalWrite(pinToGpio(_item.pin), _item.level);
}

#else

uint8_t Leds::add(uint8_t pin, bool level, ledmode_t mode) {
  if (pin > 16)
    return ERR_INDEX;

  uint8_t result;
  _led_t l;

  l.pin = (pin == 16) ? GPIO16_RENUM : pin;
  l.level = level;
  l.mode = mode;
  result = List<_led_t, 10>::add(l);
  if (result != ERR_INDEX) {
    pinMode(pin, OUTPUT);
    setMode(result, mode);
  }

  return result;
}

ledmode_t Leds::getMode(uint8_t index) const {
  if (_items && (index < _count)) {
    return _items[index].mode;
  }
}

void Leds::setMode(uint8_t index, ledmode_t mode) {
  if (_items && (index < _count)) {
    _items[index].mode = mode;
    update(index, true);
  }
}

void Leds::update(uint8_t index, bool force) {
  if (_items) {
    uint8_t i;

    if (index < _count)
      i = index;
    else if (index == ERR_INDEX)
      i = 0;
    else
      return;
    while (i < _count) {
      if (force || (_items[i].mode > LED_ON)) {
        if (_items[i].mode == LED_OFF) {
          off(i);
        } else if (_items[i].mode == LED_ON) {
          on(i);
        } else {
          uint16_t subsec;

          if (_items[i].mode == LED_FADEINOUT)
            subsec = millis() % 2000;
          else
            subsec = millis() % 1000;
          if (_items[i].mode == LED_1HZ) {
            if (subsec < BLINK_TIME)
              on(i);
            else
              off(i);
          } else if (_items[i].mode == LED_2HZ) {
            if (subsec < BLINK_TIME)
              on(i);
            else if (subsec < 500)
              off(i);
            else if (subsec < 500 + BLINK_TIME)
              on(i);
            else
              off(i);
          } else if (_items[i].mode == LED_4HZ) {
            if (subsec < BLINK_TIME)
              on(i);
            else if (subsec < 250)
              off(i);
            else if (subsec < 250 + BLINK_TIME)
              on(i);
            else if (subsec < 500)
              off(i);
            else if (subsec < 500 + BLINK_TIME)
              on(i);
            else if (subsec < 750)
              off(i);
            else if (subsec < 750 + BLINK_TIME)
              on(i);
            else
              off(i);
          } else if (_items[i].mode == LED_FADEIN) {
            if (_items[i].level)
              analogWrite(pinToGpio(_items[i].pin), map(subsec, 0, 999, 0, 1023));
            else
              analogWrite(pinToGpio(_items[i].pin), map(subsec, 0, 999, 1023, 0));
          } else if (_items[i].mode == LED_FADEOUT) {
            if (_items[i].level)
              analogWrite(pinToGpio(_items[i].pin), map(subsec, 0, 999, 1023, 0));
            else
              analogWrite(pinToGpio(_items[i].pin), map(subsec, 0, 999, 0, 1023));
          } else if (_items[i].mode == LED_FADEINOUT) {
            if (subsec < 1000) {
              if (_items[i].level)
                analogWrite(pinToGpio(_items[i].pin), map(subsec, 0, 999, 0, 1023));
              else
                analogWrite(pinToGpio(_items[i].pin), map(subsec, 0, 999, 1023, 0));
            } else {
              if (_items[i].level)
                analogWrite((_items[i].pin), map(subsec, 1000, 1999, 1023, 0));
              else
                analogWrite((_items[i].pin), map(subsec, 1000, 1999, 0, 1023));
            }
          }
        }
      }
      if (index == ERR_INDEX)
        ++i;
      else
        break;
    }
  }
}

void Leds::delay(uint32_t ms) {
  bool updating = false;

  if (_items) {
    for (uint8_t i = 0; i < _count; ++i) {
      if (_items[i].mode > LED_ON) {
        updating = true;
        break;
      }
    }
  }
  if (updating) {
    while (ms--) {
      update();
      ::delay(1);
    }
  } else
    delay(ms);
}

bool Leds::match(uint8_t index, const void *t) {
  if (_items && (index < _count)) {
    return (_items[index].pin == ((_led_t*)t)->pin);
  }

  return false;
}

void Leds::on(uint8_t index) {
  if (_items && (index < _count)) {
    digitalWrite(pinToGpio(_items[index].pin), _items[index].level);
  }
}

void Leds::off(uint8_t index) {
  if (_items && (index < _count)) {
    digitalWrite(pinToGpio(_items[index].pin), ! _items[index].level);
  }
}

#endif
