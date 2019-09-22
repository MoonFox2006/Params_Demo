#ifdef ESP32
#include <SPIFFS.h>
#else
#include <FS.h>
#endif
#include "BaseConfig.h"
#include "StrUtils.h"

bool BaseConfig::getParam(uint8_t index, param_t &param) const {
  if (index < _paramCount) {
    memcpy_P(&param, &_params[index], sizeof(param_t));

    return true;
  }

  return false;
}

uint8_t BaseConfig::findParam(const char *name) const {
  for (uint8_t i = 0; i < _paramCount; ++i) {
    if (strcasecmp_P(name, (PGM_P)pgm_read_ptr(&_params[i]._name)) == 0)
      return i;
  }

  return ERR_INDEX;
}

uint8_t BaseConfig::findParam_P(PGM_P name) const {
  for (uint8_t i = 0; i < _paramCount; ++i) {
    if (strcasecmp_PP(name, (PGM_P)pgm_read_ptr(&_params[i]._name)) == 0)
      return i;
  }

  return ERR_INDEX;
}

paramtype_t BaseConfig::paramType(uint8_t index) const {
  if (index < _paramCount) {
    return (paramtype_t)pgm_read_byte(&_params[index]._type);
  }
}

PGM_P BaseConfig::paramName(uint8_t index) const {
  if (index < _paramCount) {
    return (PGM_P)pgm_read_ptr(&_params[index]._name);
  }

  return NULL;
}

PGM_P BaseConfig::paramDescr(uint8_t index) const {
  if (index < _paramCount) {
    return (PGM_P)pgm_read_ptr(&_params[index]._descr);
  }

  return NULL;
}

uint16_t BaseConfig::paramSize(uint8_t index) const {
  if (index < _paramCount) {
    return pgm_read_word(&_params[index]._size);
  }

  return 0;
}

void BaseConfig::clear() {
  for (uint8_t i = 0; i < _paramCount; ++i) {
    void *value = getParamPtr(i);

    if (value) {
      uint16_t parsize = pgm_read_word(&_params[i]._size);

      if (parsize) {
        paramtype_t partype = (paramtype_t)pgm_read_byte(&_params[i]._type);

        if ((partype == PAR_STR) || (partype == PAR_PSWD)) {
          memset(value, 0, parsize);
          if (pgm_read_ptr(&_params[i]._default.asstr))
            strncpy_P((char*)value, (PGM_P)pgm_read_ptr(&_params[i]._default.asstr), parsize - 1);
        } else {
          memcpy_P(value, &_params[i]._default, parsize);
        }
      }
    }
  }
}

bool BaseConfig::load() {
  char mode[2];

  mode[0] = 'r';
  mode[1] = '\0';

  File file = SPIFFS.open(FPSTR(CONFIG_FILE_NAME), mode);

  if (file) {
    DynamicJsonDocument jsonDoc(JSON_BUF_SIZE);
    DeserializationError error = deserializeJson(jsonDoc, file);

    file.close();
    if (! error) {
      read(jsonDoc);

      return true;
    }
  }

  return false;
}

bool BaseConfig::save() {
  char mode[2];

  mode[0] = 'w';
  mode[1] = '\0';

  File file = SPIFFS.open(FPSTR(CONFIG_FILE_NAME), mode);

  if (file) {
    DynamicJsonDocument jsonDoc(JSON_BUF_SIZE);

    write(jsonDoc);
    serializeJson(jsonDoc, file);
    file.close();

    return true;
  }

  return false;
}

String BaseConfig::toString() {
  String result;
  DynamicJsonDocument jsonDoc(JSON_BUF_SIZE);

  write(jsonDoc);
  serializeJsonPretty(jsonDoc, result);

  return result;
}

bool BaseConfig::fromString(const String &str) {
  DynamicJsonDocument jsonDoc(JSON_BUF_SIZE);
  DeserializationError error = deserializeJson(jsonDoc, str);

  if (! error) {
    read(jsonDoc);

    return true;
  }

  return false;
}

void BaseConfig::read(const JsonDocument &doc) {
  for (uint8_t i = 0; i < _paramCount; ++i) {
    void *value = getParamPtr(i);

    if (value) {
      uint16_t parsize = pgm_read_word(&_params[i]._size);

      if (parsize) {
        paramtype_t partype = (paramtype_t)pgm_read_byte(&_params[i]._type);
        PGM_P parname = (PGM_P)pgm_read_ptr(&_params[i]._name);

        if (doc.containsKey(FPSTR(parname))) {
          if (partype == PAR_BOOL)
            *(bool*)value = doc[FPSTR(parname)].as<bool>();
          else if (partype == PAR_I8)
            *(int8_t*)value = doc[FPSTR(parname)].as<int8_t>();
          else if (partype == PAR_UI8)
            *(uint8_t*)value = doc[FPSTR(parname)].as<uint8_t>();
          else if (partype == PAR_I16)
            *(int16_t*)value = doc[FPSTR(parname)].as<int16_t>();
          else if (partype == PAR_UI16)
            *(uint16_t*)value = doc[FPSTR(parname)].as<uint16_t>();
          else if (partype == PAR_I32)
            *(int32_t*)value = doc[FPSTR(parname)].as<int32_t>();
          else if (partype == PAR_UI32)
            *(uint32_t*)value = doc[FPSTR(parname)].as<uint32_t>();
          else if (partype == PAR_FLOAT)
            *(float*)value = doc[FPSTR(parname)].as<float>();
          else if (partype == PAR_CHAR)
            *(char*)value = doc[FPSTR(parname)].as<char>();
          else if ((partype == PAR_STR) || (partype == PAR_PSWD)) {
            memset(value, 0, parsize);
            strncpy((char*)value, doc[FPSTR(parname)].as<const char*>(), parsize - 1);
          }
        } else {
          if ((partype == PAR_STR) || (partype == PAR_PSWD)) {
            memset(value, 0, parsize);
            if (pgm_read_ptr(&_params[i]._default.asstr))
              strncpy_P((char*)value, (PGM_P)pgm_read_ptr(&_params[i]._default.asstr), parsize - 1);
          } else {
            memcpy_P(value, &_params[i]._default, parsize);
          }
        }
      }
    }
  }
}

void BaseConfig::write(JsonDocument &doc) {
  for (uint8_t i = 0; i < _paramCount; ++i) {
    void *value = getParamPtr(i);

    if (value) {
      uint16_t parsize = pgm_read_word(&_params[i]._size);

      if (parsize) {
        paramtype_t partype = (paramtype_t)pgm_read_byte(&_params[i]._type);
        PGM_P parname = (PGM_P)pgm_read_ptr(&_params[i]._name);

        if (partype == PAR_BOOL)
          doc[FPSTR(parname)] = *(bool*)value;
        else if (partype == PAR_I8)
          doc[FPSTR(parname)] = *(int8_t*)value;
        else if (partype == PAR_UI8)
          doc[FPSTR(parname)] = *(uint8_t*)value;
        else if (partype == PAR_I16)
          doc[FPSTR(parname)] = *(int16_t*)value;
        else if (partype == PAR_UI16)
          doc[FPSTR(parname)] = *(uint16_t*)value;
        else if (partype == PAR_I32)
          doc[FPSTR(parname)] = *(int32_t*)value;
        else if (partype == PAR_UI32)
          doc[FPSTR(parname)] = *(uint32_t*)value;
        else if (partype == PAR_FLOAT)
          doc[FPSTR(parname)] = *(float*)value;
        else if (partype == PAR_CHAR)
          doc[FPSTR(parname)] = *(char*)value;
        else if ((partype == PAR_STR) || (partype == PAR_PSWD))
          doc[FPSTR(parname)] = (char*)value;
      }
    }
  }
}

bool initSPIFFS() {
#ifdef ESP32
  return SPIFFS.begin(true);
#else
  if (! SPIFFS.begin()) {
    if ((! SPIFFS.format()) || (! SPIFFS.begin())) {
      return false;
    }
  }
  return true;
#endif
}
