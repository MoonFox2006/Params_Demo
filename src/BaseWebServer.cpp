#ifdef ESP32
#include <SPIFFS.h>
#include <Update.h>
#else
#include <FS.h>
#include <WiFiUdp.h>
#endif
#include "BaseWebServer.h"
#include "StrUtils.h"
#include "HtmlHelper.h"

static const char HTML_CONFIG_PARAM[] PROGMEM = "config";
static const char HTML_COMPLEX_PARAM[] PROGMEM = "complex";

static const char JSON_TYPE_PARAM[] PROGMEM = "t";
static const char JSON_VALUE_PARAM[] PROGMEM = "v";
static const char JSON_DESCR_PARAM[] PROGMEM = "d";
static const char JSON_SIZE_PARAM[] PROGMEM = "s";

static const char JSON_TYPES[][3] PROGMEM = { "B", "I1", "U1", "I2", "U2", "I4", "U4", "F", "C", "S", "P" }; // paramtype_t as index

static const char FALSE[] PROGMEM = "false";
static const char TRUE[] PROGMEM = "true";

bool BaseWebServer::_setup() {
#ifdef ESP32
  _http = new WebServer(80);
#else
  _http = new ESP8266WebServer(80);
#endif
  if (! _http)
    return false;

  setupHandles();
  begin();

  return true;
}

void BaseWebServer::_loop() {
  if (_http) {
    _http->handleClient();
  }
}

void BaseWebServer::begin() {
  if (_http) {
    _http->begin();
  }
}

void BaseWebServer::cleanup() {
#ifdef USE_SERIAL
  Serial.flush();
#endif
}

void BaseWebServer::restart() {
#ifdef USE_SERIAL
  Serial.println();
  Serial.println(F("System restarting..."));
#endif
  cleanup();
  ESP.restart();
}

void BaseWebServer::setupHandles() {
  _http->onNotFound([this]() { this->handleNotFound(); });
  _http->on(FPSTR(CSS_URI), HTTP_GET, [this]() { this->handleCss(); });
  _http->on(FPSTR(SCRIPT_URI), HTTP_GET, [this]() { this->handleScript(); });
  _http->on(FPSTR(ROOT_URI), HTTP_GET, [this]() { this->handleRoot(); });
  _http->on(FPSTR(SETUP_URI), HTTP_GET, [this]() { this->handleSetup(); });
  _http->on(FPSTR(CONFIG_URI), HTTP_GET, [this]() { this->handleGetConfig(); });
  _http->on(FPSTR(CONFIG_URI), HTTP_POST, [this]() { this->handleSetConfig(); });
  _http->on(FPSTR(CONFIG_URI), HTTP_DELETE, [this]() { this->handleClearConfig(); });
  _http->on(FPSTR(RESTART_URI), HTTP_GET, [this]() { this->handleRestart(); });
  _http->on(FPSTR(SPIFFS_URI), HTTP_GET, [this]() { this->handleSPIFFS(); });
  _http->on(FPSTR(SPIFFS_URI), HTTP_POST, [this]() { this->handleFileUploaded(); }, [this]() { this->handleFileUpload(); });
  _http->on(FPSTR(SPIFFS_URI), HTTP_DELETE, [this]() { this->handleFileDelete(); });
  _http->on(FPSTR(FWUPDATE_URI), HTTP_GET, [this]() { this->handleFwUpdate(); });
  _http->on(FPSTR(FWUPDATE_URI), HTTP_POST, [this]() { this->handleSketchUpdated(); }, [this]() { this->handleSketchUpdate(); });
}

void BaseWebServer::handleNotFound() {
  if (! beforeHandle())
    return;

  if (! handleFileRead(_http->uri()))
    _http->send_P(404, TEXT_PLAIN, PSTR("Page not found!"));
}

void BaseWebServer::handleCss() {
  if (! beforeHandle())
    return;

  if (! handleFileRead(_http->uri())) {
    _http->send_P(200, TEXT_CSS, PSTR("body{background-color:rgb(240,240,240);}"));
  }
}

void BaseWebServer::handleScript() {
  if (! beforeHandle())
    return;

  _http->send_P(200, APPLICATION_JAVASCRIPT, PSTR("function getXmlHttpRequest(){\n"
    "var xmlhttp;\n"
    "try{\n"
    "xmlhttp=new ActiveXObject('Msxml2.XMLHTTP');\n"
    "}catch(e){\n"
    "try{\n"
    "xmlhttp=new ActiveXObject('Microsoft.XMLHTTP');\n"
    "}catch(E){\n"
    "xmlhttp=false;\n"
    "}\n"
    "}\n"
    "if((!xmlhttp)&&(typeof XMLHttpRequest!='undefined')){\n"
    "xmlhttp=new XMLHttpRequest();\n"
    "}\n"
    "return xmlhttp;\n"
    "}\n"
    "function urlGet(url){\n"
    "var request=getXmlHttpRequest();\n"
    "request.open('GET',url,false);\n"
    "request.send(null);\n"
    "if(request.status==200)\n"
    "return request.responseText;\n"
    "return null;\n"
    "}\n"
    "function urlPost(url,payload){\n"
    "var request=getXmlHttpRequest();\n"
    "request.open('POST',url,false);\n"
    "request.send(payload);\n"
    "if(request.status==200)\n"
    "return request.responseText;\n"
    "return null;\n"
    "}\n"
    "function urlDelete(url){\n"
    "var request=getXmlHttpRequest();\n"
    "request.open('DELETE',url,false);\n"
    "request.send(null);\n"
    "if(request.status==200)\n"
    "return request.responseText;\n"
    "return null;\n"
    "}"));
}

void BaseWebServer::handleRoot() {
  if (! beforeHandle())
    return;

  String page = FPSTR(HTML_PAGE_START);

  page += tag_P(PSTR("title"), F("Web Application"), true);
  page += getCss();
  page += FPSTR(HTML_HEAD_END);
  page += FPSTR(HTML_BODY_START);
  page += F("<button onclick=\"location.href='");
  page += FPSTR(SETUP_URI);
  page += F("'\">Setup</button>\n"
    "<button onclick=\"location.href='");
  page += FPSTR(RESTART_URI);
  page += F("'\">Restart!</button>\n");
  page += FPSTR(HTML_PAGE_END);
  _http->send(200, FPSTR(TEXT_HTML), page);
}

void BaseWebServer::handleSetup() {
  if (! beforeHandle())
    return;

  static const char ISINT_PARAM[] PROGMEM = "isInt";
  static const char ISFLOAT_PARAM[] PROGMEM = "isFloat";

  String page = FPSTR(HTML_PAGE_START);

  page += tag_P(PSTR("title"), F("Edit config"), true);
  page += F("<script type=\"");
  page += FPSTR(APPLICATION_JAVASCRIPT);
  page += F("\" src=\"");
  page += FPSTR(SCRIPT_URI);
  page += F("\"></script>\n");
  page += F("<script type=\"");
  page += FPSTR(APPLICATION_JAVASCRIPT);
  page += F("\">\n"
    "function load(form){\n"
    "try{\n"
    "var config=JSON.parse(urlGet('");
  page += FPSTR(CONFIG_URI);
  page += '?';
  page += FPSTR(HTML_COMPLEX_PARAM);
  page += F("&dummy='+Date.now()));\n"
    "var table, tr, td, elem;\n"
    "table=document.getElementById('table');\n"
    "for(var name in config){\n"
    "tr=table.insertRow(-1);\n"
    "td=tr.insertCell(0);\n"
    "td.align='right';\n"
    "if(config[name].");
  page += FPSTR(JSON_DESCR_PARAM);
  page += F(")\n"
    "elem=document.createTextNode(config[name].");
  page += FPSTR(JSON_DESCR_PARAM);
  page += F(");\n"
    "else\n"
    "elem=document.createTextNode(name);\n"
    "td.appendChild(elem);\n"
    "td=tr.insertCell(1);\n"
    "elem=document.createElement('input');\n"
    "elem.name=name;\n"
    "if(config[name].");
  page += FPSTR(JSON_TYPE_PARAM);
  page += F("=='");
  page += FPSTR(JSON_TYPES[PAR_BOOL]);
  page += F("'){\n"
    "elem.type='checkbox';\n"
    "elem.checked=config[name].");
  page += FPSTR(JSON_VALUE_PARAM);
  page += F(";\n"
    "}else if(config[name].");
  page += FPSTR(JSON_TYPE_PARAM);
  page += F("=='");
  page += FPSTR(JSON_TYPES[PAR_PSWD]);
  page += F("'){\n"
    "elem.type='password';\n"
    "elem.value=config[name].");
  page += FPSTR(JSON_VALUE_PARAM);
  page += F(";\n"
    "elem.size=config[name].");
  page += FPSTR(JSON_SIZE_PARAM);
  page += F("-1;\n"
    "elem.maxLength=elem.size;\n"
    "}else{\n"
    "elem.type='text';\n"
    "elem.value=config[name].");
  page += FPSTR(JSON_VALUE_PARAM);
  page += F(";\n"
    "switch(config[name].");
  page += FPSTR(JSON_TYPE_PARAM);
  page += F("){\n"
    "case '");
  page += FPSTR(JSON_TYPES[PAR_FLOAT]);
  page += F("':\n"
    "elem.size=15;\n"
    "elem.");
  page += FPSTR(ISFLOAT_PARAM);
  page += F("=true;\n"
    "break;\n"
    "case '");
  page += FPSTR(JSON_TYPES[PAR_I8]);
  page += F("':\n"
    "elem.size=4;\n"
    "elem.");
  page += FPSTR(ISINT_PARAM);
  page += F("=true;\n"
    "break;\n"
    "case '");
  page += FPSTR(JSON_TYPES[PAR_UI8]);
  page += F("':\n"
    "elem.size=3;\n"
    "elem.");
  page += FPSTR(ISINT_PARAM);
  page += F("=true;\n"
    "break;\n"
    "case '");
  page += FPSTR(JSON_TYPES[PAR_I16]);
  page += F("':\n"
    "elem.size=6;\n"
    "elem.");
  page += FPSTR(ISINT_PARAM);
  page += F("=true;\n"
    "break;\n"
    "case '");
  page += FPSTR(JSON_TYPES[PAR_UI16]);
  page += F("':\n"
    "elem.size=5;\n"
    "elem.");
  page += FPSTR(ISINT_PARAM);
  page += F("=true;\n"
    "break;\n"
    "case '");
  page += FPSTR(JSON_TYPES[PAR_I32]);
  page += F("':\n"
    "elem.size=11;\n"
    "elem.");
  page += FPSTR(ISINT_PARAM);
  page += F("=true;\n"
    "break;\n"
    "case '");
  page += FPSTR(JSON_TYPES[PAR_UI32]);
  page += F("':\n"
    "elem.size=10;\n"
    "elem.");
  page += FPSTR(ISINT_PARAM);
  page += F("=true;\n"
    "break;\n"
    "case '");
  page += FPSTR(JSON_TYPES[PAR_CHAR]);
  page += F("':\n"
    "elem.size=1;\n"
    "break;\n"
    "default:\n"
    "elem.size=config[name].");
  page += FPSTR(JSON_SIZE_PARAM);
  page += F("-1;\n"
    "}\n"
    "elem.maxLength=elem.size;\n"
    "}\n"
    "td.appendChild(elem);\n"
    "}\n"
    "return true;\n"
    "}catch(e){\n"
    "alert('Exception '+e.name+': '+e.message);\n"
    "return false;\n"
    "}\n"
    "}\n"

    "function store(form){\n"
    "try{\n"
    "var config={};\n"
    "var table=document.getElementById('table');\n"
    "for(var i=0;i<table.rows.length;++i){\n"
    "var elements=table.rows[i].cells[1].getElementsByTagName('input');\n"
    "for(var j=0;j<elements.length;++j){\n"
    "if(elements[j].type=='checkbox'){\n"
    "config[elements[j].name]=elements[j].checked;\n"
    "elements[j].disabled=true;\n"
    "}else if((elements[j].type=='text')||(elements[j].type=='password')){\n"
    "if(elements[j].");
  page += FPSTR(ISFLOAT_PARAM);
  page += F(")\n"
    "config[elements[j].name]=parseFloat(elements[j].value);\n"
    "else if(elements[j].");
  page += FPSTR(ISINT_PARAM);
  page += F(")\n"
    "config[elements[j].name]=parseInt(elements[j].value);\n"
    "else\n"
    "config[elements[j].name]=elements[j].value;\n"
    "elements[j].disabled=true;\n"
    "}\n"
    "}\n"
    "}\n"
    "form.");
  page += FPSTR(HTML_CONFIG_PARAM);
  page += F(".value=JSON.stringify(config);\n"
    "return true;\n"
    "}catch(e){\n"
    "alert('Exception '+e.name+': '+e.message);\n"
    "return false;\n"
    "}\n"
    "}\n"
    "</script>\n");
  page += getCss();
  page += FPSTR(HTML_HEAD_END);
  page += F("<body onload=\"load(form)\">\n"
    "<form name=\"form\" action=\"");
  page += FPSTR(CONFIG_URI);
  page += F("\" method=\"POST\" onsubmit=\"store(this)\">\n"
    "<b>Configuration:</b>\n"
    "<table id=\"table\" cols=2>\n"
    "</table>\n"
    "<input type=\"hidden\" name=\"");
  page += FPSTR(HTML_CONFIG_PARAM);
  page += F("\">\n"
    "<input type=\"submit\" value=\"Store\">\n"
    "<input type=\"button\" value=\"Clear\" onclick=\"if(urlDelete('");
  page += FPSTR(CONFIG_URI);
  page += F("')!==null) location.reload()\">\n"
    "<input type=\"button\" value=\"Restart!\" onclick=\"location.href='");
  page += FPSTR(RESTART_URI);
  page += F("'\">\n");
  page += FPSTR(HTML_PAGE_END);
  _http->send(200, FPSTR(TEXT_HTML), page);
}

void BaseWebServer::handleGetConfig() {
  if (! beforeHandle())
    return;

  static const char COMMA_QUOTE[] PROGMEM = ",\"";
  static const char QUOTE_COMMA_QUOTE[] PROGMEM = "\",\"";
  static const char QUOTE_COLON[] PROGMEM = "\":";
  static const char QUOTE_COLON_QUOTE[] PROGMEM = "\":\"";

  bool complex = _http->hasArg(FPSTR(HTML_COMPLEX_PARAM));
  String page = F("{");

  for (uint8_t i = 0; i < _config->paramCount(); ++i) {
    void *value = _config->getParamPtr(i);

    if (value) {
      uint16_t parsize = _config->paramSize(i);

      if (parsize) {
        paramtype_t partype = _config->paramType(i);

        if (page.length() > 1) // Not first
          page += FPSTR(COMMA_QUOTE);
        else
          page += '"';
        page += FPSTR(_config->paramName(i));
        page += FPSTR(QUOTE_COLON);
        if (complex) {
          page += F("{\"");
          page += FPSTR(JSON_TYPE_PARAM);
          page += FPSTR(QUOTE_COLON_QUOTE);
          page += FPSTR(JSON_TYPES[partype]);
          page += FPSTR(QUOTE_COMMA_QUOTE);
          page += FPSTR(JSON_VALUE_PARAM);
          page += FPSTR(QUOTE_COLON);
        }
        if ((partype == PAR_CHAR) || (partype == PAR_STR) || (partype == PAR_PSWD)) {
          page += '"';
          if (partype == PAR_CHAR)
            page += *(char*)value;
          else
            page += (char*)value;
          page += '"';
        } else {
          if (partype == PAR_BOOL) {
            if (*(bool*)value)
              page += FPSTR(TRUE);
            else
              page += FPSTR(FALSE);
          } else if (partype == PAR_I8)
            page += String(*(int8_t*)value);
          else if (partype == PAR_UI8)
            page += String(*(uint8_t*)value);
          else if (partype == PAR_I16)
            page += String(*(int16_t*)value);
          else if (partype == PAR_UI16)
            page += String(*(uint16_t*)value);
          else if (partype == PAR_I32)
            page += String(*(int32_t*)value);
          else if (partype == PAR_UI32)
            page += String(*(uint32_t*)value);
          else if (partype == PAR_FLOAT)
            page += String(*(float*)value);
        }
        if (complex) {
          PGM_P descr = _config->paramDescr(i);

          if (descr) {
            page += FPSTR(COMMA_QUOTE);
            page += FPSTR(JSON_DESCR_PARAM);
            page += FPSTR(QUOTE_COLON_QUOTE);
            page += FPSTR(descr);
            page += '"';
          }
          if ((partype == PAR_STR) || (partype == PAR_PSWD)) {
            page += FPSTR(COMMA_QUOTE);
            page += FPSTR(JSON_SIZE_PARAM);
            page += FPSTR(QUOTE_COLON);
            page += String(parsize);
          }
          page += '}';
        }
      }
    }
  }
  page += '}';
  _http->send(200, FPSTR(APPLICATION_JSON), page);
}

void BaseWebServer::handleSetConfig() {
  if (! beforeHandle())
    return;

  uint16_t retcode = 400;
  String page = FPSTR(HTML_PAGE_START);

  page += tag_P(PSTR("title"), F("Store config"), true);
  page += F("<meta http-equiv=\"refresh\" content=\"2;URL=");
  page += FPSTR(SETUP_URI);
  page += F("\">\n");
  page += FPSTR(HTML_HEAD_END);
  page += FPSTR(HTML_BODY_START);
  if (_http->hasArg(FPSTR(HTML_CONFIG_PARAM))) {
    if (_config->fromString(_http->arg(FPSTR(HTML_CONFIG_PARAM)))) {
      if (_config->save()) {
        retcode = 200;
        page += F("OK\n");
        Serial.println(F("Config updated successfully"));
      } else {
        page += F("Store error!\n");
        Serial.println(F("Error updating config!"));
      }
    } else {
      page += F("Parse error!\n");
      Serial.println(F("Error parsing config!"));
    }
  } else {
    page += F("Missing parameter!\n");
    Serial.println(F("Missing parameter!"));
  }
  page += FPSTR(HTML_PAGE_END);
  _http->send(retcode, FPSTR(TEXT_HTML), page);
}

void BaseWebServer::handleClearConfig() {
  if (! beforeHandle())
    return;

  uint16_t retcode = 400;
  String page = FPSTR(HTML_PAGE_START);

  page += tag_P(PSTR("title"), F("Clear config"), true);
  page += F("<meta http-equiv=\"refresh\" content=\"2;URL=");
  page += FPSTR(SETUP_URI);
  page += F("\">\n");
  page += FPSTR(HTML_HEAD_END);
  page += FPSTR(HTML_BODY_START);
  _config->clear();
  if (_config->save()) {
    retcode = 200;
    page += F("OK\n");
    Serial.println(F("Config cleared successfully"));
  } else {
    page += F("Clear error!\n");
    Serial.println(F("Error clearing config!"));
  }
  page += FPSTR(HTML_PAGE_END);
  _http->send(retcode, FPSTR(TEXT_HTML), page);
}

void BaseWebServer::handleRestart() {
  if (! beforeHandle())
    return;

  _http->send_P(200, TEXT_PLAIN, PSTR("Restarting..."));
  _http->close();

  restart();
}

void BaseWebServer::handleSPIFFS() {
  if (! beforeHandle())
    return;

  String page = FPSTR(HTML_PAGE_START);

  page += tag_P(PSTR("title"), F("SPIFFS"), true);
  page += F("<script type=\"");
  page += FPSTR(APPLICATION_JAVASCRIPT);
  page += F("\" src=\"");
  page += FPSTR(SCRIPT_URI);
  page += F("\"></script>\n");
  page += F("<script type=\"");
  page += FPSTR(APPLICATION_JAVASCRIPT);
  page += F("\">\n"
    "function getSelectedCount(){\n"
    "var inputs=document.getElementsByTagName('input');\n"
    "var result=0;\n"
    "for(var i=0;i<inputs.length;++i){\n"
    "if(inputs[i].type=='checkbox'){\n"
    "if(inputs[i].checked)\n"
    "++result;\n"
    "}\n"
    "}\n"
    "return result;\n"
    "}\n"
    "function updateSelected(){\n"
    "document.getElementsByName('delete')[0].disabled=getSelectedCount()==0;\n"
    "}\n"
    "function deleteSelected(){\n"
    "var inputs=document.getElementsByTagName('input');\n"
    "for(var i=0;i<inputs.length;++i){\n"
    "if(inputs[i].type=='checkbox'){\n"
    "if(inputs[i].checked)\n"
    "if(urlDelete('");
  page += FPSTR(SPIFFS_URI);
  page += F("?filename=/'+encodeURIComponent(inputs[i].value)+'&dummy='+Date.now())===null)\n"
    "alert('Error!');\n"
    "}\n"
    "}\n"
    "location.reload(true);\n"
    "}\n"
    "</script>\n");
  page += getCss();
  page += FPSTR(HTML_HEAD_END);
  page += FPSTR(HTML_BODY_START);
  page += F("<form method=\"POST\" action=\"\" enctype=\"multipart/form-data\" onsubmit=\"if(document.getElementsByName('upload')[0].files.length==0){alert('No file to upload!');return false;}\">\n"
    "<h3>SPIFFS</h3>\n"
    "<p>\n");

#ifdef ESP32
  File dir = SPIFFS.open(FPSTR(ROOT_URI));
  File file;
#else
  Dir dir = SPIFFS.openDir(FPSTR(ROOT_URI));
#endif
  int cnt = 0;

#ifdef ESP32
  if (dir) {
#else
//  if (dir.isDirectory()) {
  {
#endif
    page += F("<table cols=2>\n");
#ifdef ESP32
    while (file = dir.openNextFile()) {
#else
    while (dir.next()) {
#endif
      String fileName;
      size_t fileSize;

      ++cnt;
#ifdef ESP32
      fileName = file.name();
      fileSize = file.size();
#else
      fileName = dir.fileName();
      fileSize = dir.fileSize();
#endif
      if (fileName.startsWith(FPSTR(ROOT_URI)))
        fileName = fileName.substring(1);
      page += F("<tr><td><input type=\"checkbox\" name=\"file");
      page += String(cnt);
      page += F("\" value=\"");
      page += fileName;
      page += F("\" onchange=\"updateSelected()\"><a href=\"/");
      page += fileName;
      page += F("\" download>");
      page += fileName;
      page += F("</a></td><td>");
      page += String(fileSize);
      page += F("</td></tr>\n");
    }
    page += F("</table>\n");
  }
  page += String(cnt);
  page += F(" file(s)\n"
    "<p>\n"
    "<input type=\"button\" name=\"delete\" value=\"Delete\" onclick=\"if(confirm('Are you sure to delete selected file(s)?')) deleteSelected()\" disabled>\n"
    "<p>\n"
    "Upload new file:<br/>\n"
    "<input type=\"file\" name=\"upload\">\n"
    "<input type=\"submit\" value=\"Upload\">\n"
    "</form>\n");
  page += FPSTR(HTML_PAGE_END);
  _http->send(200, FPSTR(TEXT_HTML), page);
}

void BaseWebServer::handleFileUploaded() {
  if (! beforeHandle())
    return;

  _http->send_P(200, TEXT_HTML, PSTR("<META http-equiv=\"refresh\" content=\"2;URL=\">\n"
    "Upload successful."));
}

void BaseWebServer::handleFileUpload() {
  if (! beforeHandle())
    return;

  static File uploadFile;

  if (_http->uri() != FPSTR(SPIFFS_URI))
    return;
  HTTPUpload &upload = _http->upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    char mode[2];

    if (! filename.startsWith(FPSTR(ROOT_URI)))
      filename = '/' + filename;
    mode[0] = 'w';
    mode[1] = '\0';
    uploadFile = SPIFFS.open(filename, mode);
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (uploadFile)
      uploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (uploadFile)
      uploadFile.close();
  }
}

void BaseWebServer::handleFileDelete() {
  if (! beforeHandle())
    return;

  if (! _http->args())
    return _http->send_P(500, TEXT_PLAIN, PSTR("BAD ARGS"));

  String path = _http->arg(0);
  if (path == FPSTR(ROOT_URI))
    return _http->send_P(500, TEXT_PLAIN, PSTR("BAD PATH"));
  if (! SPIFFS.exists(path))
    return _http->send_P(404, TEXT_PLAIN, PSTR("File not found!"));
  SPIFFS.remove(path);
  _http->send_P(200, TEXT_PLAIN, PSTR("OK"));
}

void BaseWebServer::handleFwUpdate() {
  if (! beforeHandle())
    return;

  String page = FPSTR(HTML_PAGE_START);

  page += tag_P(PSTR("title"), F("Sketch Update"), true);
  page += getCss();
  page += FPSTR(HTML_HEAD_END);
  page += FPSTR(HTML_BODY_START);
  page += F("<form method=\"POST\" action=\"\" enctype=\"multipart/form-data\" onsubmit=\"if(document.getElementsByName('update')[0].files.length==0){alert('No file to update!');return false;}\">\n"
    "Select compiled sketch to upload:<br/>\n"
    "<input type=\"file\" name=\"upload\">\n"
    "<input type=\"submit\" value=\"Update\">\n"
    "</form>\n");
  page += FPSTR(HTML_PAGE_END);
  _http->send(200, FPSTR(TEXT_HTML), page);
}

void BaseWebServer::handleSketchUpdated() {
  if (! beforeHandle())
    return;

  _http->send_P(200, TEXT_HTML, Update.hasError() ? PSTR("Update failed!") : PSTR("<META http-equiv=\"refresh\" content=\"15;URL=\">\nUpdate successful! Rebooting..."));
  if (! Update.hasError()) {
    _http->close();
    restart();
  }
}

void BaseWebServer::handleSketchUpdate() {
  if (! beforeHandle())
    return;

  if (_http->uri() != FPSTR(FWUPDATE_URI))
    return;

  HTTPUpload &upload = _http->upload();
  if (upload.status == UPLOAD_FILE_START) {
//    cleanup();
#ifndef ESP32
    WiFiUDP::stopAll();
#endif
#ifdef USE_SERIAL
    Serial.print(F("Update sketch from file \""));
    Serial.print(upload.filename);
    Serial.print('"');
#endif
    if (! Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)) { // start with max available size
#ifdef USE_SERIAL
      Serial.println();
      Update.printError(Serial);
#endif
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
#ifdef USE_SERIAL
    Serial.print('.');
#endif
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
#ifdef USE_SERIAL
      Serial.println();
      Update.printError(Serial);
#endif
    }
  } else if (upload.status == UPLOAD_FILE_END) {
#ifdef USE_SERIAL
    Serial.println();
#endif
    if (Update.end(true)) { // true to set the size to the current progress
#ifdef USE_SERIAL
      Serial.print(F("Updated "));
      Serial.print(upload.totalSize);
      Serial.println(F(" byte(s) successful"));
#endif
    } else {
#ifdef USE_SERIAL
      Update.printError(Serial);
#endif
    }
  } else if (upload.status == UPLOAD_FILE_ABORTED) {
    Update.end();
#ifdef USE_SERIAL
    Serial.println();
    Serial.println(F("Update was aborted!"));
#endif
  }
  yield();
}

#ifdef USE_AUTHORIZATION
bool BaseWebServer::checkAuthorization() {
  char user[sizeof(AUTH_USER)];
  char pswd[sizeof(AUTH_PSWD)];

  strcpy_P(user, PSTR(AUTH_USER));
  strcpy_P(pswd, PSTR(AUTH_PSWD));
  if (! _http->authenticate(user, pswd)) {
    _http->requestAuthentication();

    return false;
  }

  return true;
}
#endif

String BaseWebServer::getContentType(const String &fileName) {
  if (_http->hasArg(F("download")))
    return String(F("application/octet-stream"));
  else if (fileName.endsWith(F(".htm")) || fileName.endsWith(F(".html")))
    return String(FPSTR(TEXT_HTML));
  else if (fileName.endsWith(F(".css")))
    return String(FPSTR(TEXT_CSS));
  else if (fileName.endsWith(F(".js")))
    return String(FPSTR(APPLICATION_JAVASCRIPT));
  else if (fileName.endsWith(F(".png")))
    return String(F("image/png"));
  else if (fileName.endsWith(F(".gif")))
    return String(F("image/gif"));
  else if (fileName.endsWith(F(".jpg")) || fileName.endsWith(F(".jpeg")))
    return String(F("image/jpeg"));
  else if (fileName.endsWith(F(".ico")))
    return String(F("image/x-icon"));
  else if (fileName.endsWith(F(".xml")))
    return String(F("text/xml"));
  else if (fileName.endsWith(F(".pdf")))
    return String(F("application/x-pdf"));
  else if (fileName.endsWith(F(".zip")))
    return String(F("application/x-zip"));
  else if (fileName.endsWith(F(".gz")))
    return String(F("application/x-gzip"));

  return String(FPSTR(TEXT_PLAIN));
}

bool BaseWebServer::handleFileRead(const String &path) {
  String fileName = path;

  if (fileName.endsWith(FPSTR(ROOT_URI)))
    fileName += FPSTR(INDEX_HTML);
  String contentType = getContentType(fileName);
  if (SPIFFS.exists(fileName)) {
    char mode[2];

    mode[0] = 'r';
    mode[1] = '\0';
    File file = SPIFFS.open(fileName, mode);
    if (file) {
      _http->streamFile(file, contentType);
      file.close();

      return true;
    }
  }

  return false;
}

String BaseWebServer::getCss() {
  String result = F("<link rel=\"stylesheet\" href=\"");

  result += FPSTR(CSS_URI);
  result += F("\">\n");

  return result;
}
