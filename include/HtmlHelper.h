#ifndef __HTMLHELPER_H
#define __HTMLHELPER_H

#include <WString.h>

const char TEXT_HTML[] PROGMEM = "text/html";
const char TEXT_PLAIN[] PROGMEM = "text/plain";
const char TEXT_CSS[] PROGMEM = "text/css";
const char APPLICATION_JSON[] PROGMEM = "application/json";
const char APPLICATION_JAVASCRIPT[] PROGMEM = "application/javascript";

const char HTML_PAGE_START[] PROGMEM = "<!DOCTYPE html>\n"
  "<html>\n"
  "<head>\n"
//  "<meta http-equiv=\"Pragma\" content=\"no-cache\"/>\n"
//  "<meta http-equiv=\"Cache-Control\" content=\"no-cache\"/>\n"
  "<meta charset=\"utf-8\"/>\n";
const char HTML_HEAD_END[] PROGMEM = "</head>\n";
const char HTML_BODY_START[] PROGMEM = "<body>\n";
const char HTML_PAGE_END[] PROGMEM = "</body>\n"
  "</html>";

String tagOpen(const String &tagName, const String &tagOptions, bool nl = false);
String tagOpen(const String &tagName, bool nl = false);
String tagClose(const String &tagName, bool nl = false);

String tag(const String &tagName, const String &tagOptions, const String &tagValue, bool nl = false);
String tag(const String &tagName, const String &tagValue, bool nl = false);
String tag(const String &tagName, bool nl = false);

String tagOpen_P(PGM_P tagName, const String &tagOptions, bool nl = false);
String tagOpen_P(PGM_P tagName, bool nl = false);
String tagClose_P(PGM_P tagName, bool nl = false);

String tag_P(PGM_P tagName, const String &tagOptions, const String &tagValue, bool nl = false);
String tag_P(PGM_P tagName, const String &tagValue, bool nl = false);
String tag_P(PGM_P tagName, bool nl = false);

#endif
