//#include <ctype.h>
#include <stdlib.h>
#include "StrUtils.h"

bool allocStr(char **str, const char *src) {
  if (src && *src) {
    if (*str) {
      void *ptr = realloc(*str, strlen(src) + 1);

      if (! ptr)
        return false;
      *str = (char*)ptr;
    } else {
      *str = (char*)malloc(strlen(src) + 1);
      if (! *str)
        return false;
    }
    strcpy(*str, src);
  } else {
    if (*str) {
      free(*str);
      *str = NULL;
    }
  }

  return true;
}

bool allocStr_P(char **str, PGM_P src) {
  if (src && pgm_read_byte(src)) {
    if (*str) {
      void *ptr = realloc(*str, strlen_P(src) + 1);

      if (! ptr)
        return false;
      *str = (char*)ptr;
    } else {
      *str = (char*)malloc(strlen_P(src) + 1);
      if (! *str)
        return false;
    }
    strcpy_P(*str, src);
  } else {
    if (*str) {
      free(*str);
      *str = NULL;
    }
  }

  return true;
}

void disposeStr(char **str) {
  if (*str) {
    free(*str);
    *str = NULL;
  }
}

int8_t strcmp_PP(PGM_P s1, PGM_P s2) {
  char c1, c2;

  do {
    c1 = pgm_read_byte(s1++);
    c2 = pgm_read_byte(s2++);
  } while ((c1 == c2) && (c1 != '\0'));

  return (c1 - c2);
}

int8_t strncmp_PP(PGM_P s1, PGM_P s2, uint16_t maxlen) {
  char c1, c2;

  do {
    c1 = pgm_read_byte(s1++);
    c2 = pgm_read_byte(s2++);
  } while ((maxlen--) && (c1 == c2) && (c1 != '\0'));

  return (c1 - c2);
}

int8_t strcasecmp_PP(PGM_P s1, PGM_P s2) {
  char c1, c2;

  do {
    c1 = toupper(pgm_read_byte(s1++));
    c2 = toupper(pgm_read_byte(s2++));
  } while ((c1 == c2) && (c1 != '\0'));

  return (c1 - c2);
}

int8_t strncasecmp_PP(PGM_P s1, PGM_P s2, uint16_t maxlen) {
  char c1, c2;

  do {
    c1 = toupper(pgm_read_byte(s1++));
    c2 = toupper(pgm_read_byte(s2++));
  } while ((maxlen--) && (c1 == c2) && (c1 != '\0'));

  return (c1 - c2);
}

char *byteToHex(char *out, uint8_t value) {
  uint8_t b;

  b = value >> 4;
  if (b < 10)
    out[0] = '0' + b;
  else
    out[0] = 'A' + (b - 10);
  b = value & 0x0F;
  if (b < 10)
    out[1] = '0' + b;
  else
    out[1] = 'A' + (b - 10);
  out[2] = '\0';

  return out;
}
