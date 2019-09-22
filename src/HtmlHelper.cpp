#include "HtmlHelper.h"

String tagOpen(const String &tagName, const String &tagOptions, bool nl) {
  String result;

  result = '<';
  result += tagName;
  result += ' ';
  result += tagOptions;
  result += '>';
  if (nl)
    result += '\n';

  return result;
}

String tagOpen(const String &tagName, bool nl) {
  String result;

  result = '<';
  result += tagName;
  result += '>';
  if (nl)
    result += '\n';

  return result;
}

String tagClose(const String &tagName, bool nl) {
  String result;

  result = F("</");
  result += tagName;
  result += '>';
  if (nl)
    result += '\n';

  return result;
}

String tag(const String &tagName, const String &tagOptions, const String &tagValue, bool nl) {
  String result;

  result = '<';
  result += tagName;
  result += ' ';
  result += tagOptions;
  result += '>';
  result += tagValue;
  result += F("</");
  result += tagName;
  result += '>';
  if (nl)
    result += '\n';

  return result;
}

String tag(const String &tagName, const String &tagValue, bool nl) {
  String result;

  result = '<';
  result += tagName;
  result += '>';
  result += tagValue;
  result += F("</");
  result += tagName;
  result += '>';
  if (nl)
    result += '\n';

  return result;
}

String tag(const String &tagName, bool nl) {
  String result;

  result = '<';
  result += tagName;
  result += F("/>");
  if (nl)
    result += '\n';

  return result;
}

String tagOpen_P(PGM_P tagName, const String &tagOptions, bool nl) {
  String result;

  result = '<';
  result += FPSTR(tagName);
  result += ' ';
  result += tagOptions;
  result += '>';
  if (nl)
    result += '\n';

  return result;
}

String tagOpen_P(PGM_P tagName, bool nl) {
  String result;

  result = '<';
  result += FPSTR(tagName);
  result += '>';
  if (nl)
    result += '\n';

  return result;
}

String tagClose_P(PGM_P tagName, bool nl) {
  String result;

  result = F("</");
  result += FPSTR(tagName);
  result += '>';
  if (nl)
    result += '\n';

  return result;
}

String tag_P(PGM_P tagName, const String &tagOptions, const String &tagValue, bool nl) {
  String result;

  result = '<';
  result += FPSTR(tagName);
  result += ' ';
  result += tagOptions;
  result += '>';
  result += tagValue;
  result += F("</");
  result += FPSTR(tagName);
  result += '>';
  if (nl)
    result += '\n';

  return result;
}

String tag_P(PGM_P tagName, const String &tagValue, bool nl) {
  String result;

  result = '<';
  result += FPSTR(tagName);
  result += '>';
  result += tagValue;
  result += F("</");
  result += FPSTR(tagName);
  result += '>';
  if (nl)
    result += '\n';

  return result;
}

String tag_P(PGM_P tagName, bool nl) {
  String result;

  result = '<';
  result += FPSTR(tagName);
  result += F("/>");
  if (nl)
    result += '\n';

  return result;
}
