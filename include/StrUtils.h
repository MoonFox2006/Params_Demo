#ifndef __STRUTILS_H
#define __STRUTILS_H

#include <inttypes.h>
#include <pgmspace.h>
#include <WString.h>

bool allocStr(char **str, const char *src);
bool allocStr_P(char **str, PGM_P src);
inline bool allocStr(char **str, const __FlashStringHelper *src);
void disposeStr(char **str);

int8_t strcmp_PP(PGM_P s1, PGM_P s2);
int8_t strncmp_PP(PGM_P s1, PGM_P s2, uint16_t maxlen);
int8_t strcasecmp_PP(PGM_P s1, PGM_P s2);
int8_t strncasecmp_PP(PGM_P s1, PGM_P s2, uint16_t maxlen);

char *byteToHex(char *out, uint8_t value);

#endif
