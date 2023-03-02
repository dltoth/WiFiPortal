/**
 * 
 *  WiFiPortal Library
 *  Copyright (C) 2023  Daniel L Toth
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published 
 *  by the Free Software Foundation, either version 3 of the License, or any 
 *  later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  
 *  The author can be contacted at dan@leelanausoftware.com  
 *
 */
 
#include "WiFiPortal.h"
#include "PortalProgmem.h"

namespace lsc {

#define DISPLAY_SIZE 1280

/**
 *    In each of these templates, the "form cancel path" is either the main portal page or Access Point display page for re-home.
 *    The Cancel path invokes the application defined cancel handler.
 *    
 */
const char APwifi_button[]      PROGMEM = "<a href=\"%s?ssid=%s\" class=\"scaled apButton\">%s</a>";                                                     // apForm path, SSID, SSID
const char APcancel_button[]    PROGMEM = "<br><div align=\"center\"><a href=\"%s\" class=\"medium apButton\">"
                                                                     "Cancel</a></div>";                                                                 // Cancel path
const char AP_success[]         PROGMEM = "<!DOCTYPE html>"
                                             "<html>"
                                                "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
                                                "<body style=\"font-family: Arial\">"
                                                   "<H1 align=\"center\"> Connection Successful! Bye </H1><br>"
                                                "</body>"
                                             "</html>";
/**
 *  Start MDNS with the soft AP name provided. Abstracted for ESP8266 and ESP32.
 */
bool WiFiPortal::startMDNS() {return _mDNS.begin(_apName);}

void WiFiPortal::stopMDNS() {
#ifdef ESP8266
  _mDNS.close();
#elif defined(ESP32)
  _mDNS.end();
#endif
}

void WiFiPortal::updateMDNS() {
#ifdef ESP8266
  _mDNS.update();
#endif  
}

/**
 *   ConnectionState remains CNX_DISCONNECTED until the portal returns from a successful connection to an SSID (in finishConnect()),
 *   at which point the state is set to CNX_FINISHED.
 */
int WiFiPortal::connectWiFi() {
  if( finishedState() ) {
    if(loggingLevel(FINE)) Serial.printf_P(PSTR("WiFiPortal::connectWiFi: Connecting to %s\n"),ssid());
    if( WiFi.status() == WL_CONNECTED ) {
      setConnectionState(CNX_CONNECTED);
      if(loggingLevel(FINE)) Serial.printf_P(PSTR("connectWiFi: Connection to %s SUCCESSFUL\n"),ssid());
      finish();
    }
/**
 *   Should NOT happen   
 */
    else {
      if(loggingLevel(WARNING)) {
          Serial.printf_P(PSTR("WiFiPortal::connectWiFi: WARNING state is FINISHED but WiFi state is %s\n"),StatusStrings::wifiStatus());
      }
      resetAP();         
      setConnectionState(CNX_DISCONNECTED);
    }
  }
  else {
    updateMDNS();
    _server.handleClient();
  }
  return getConnectionState();
}

int  WiFiPortal::attemptConnect(String ssid, String psk) {
  if( (WiFi.status() != WL_CONNECTED) ) {
    if( loggingLevel(FINE) ) Serial.printf_P(PSTR("WiFiPortal::attemptConnect: Connecting to %s with %s\n"),ssid.c_str(),psk.c_str());
    WiFi.begin(ssid.c_str(),psk.c_str());
    WiFi.setAutoConnect(true);
    long startTime = millis();
    WiFi.waitForConnectResult(cnxTimeout());
    long executionTime = millis() - startTime;
    if( loggingLevel(FINE) ) {
      Serial.printf_P(PSTR("                            Connection sequence completed in %d milliseconds, WiFi status is %s\n"),executionTime,StatusStrings::wifiStatus());
    }
    if(WiFi.status() == WL_CONNECTED) { 
      if( loggingLevel(INFO) ) {
        Serial.printf_P(PSTR("                            Connection to %s successful\n"),ssid.c_str());
      }
    }
    else {
      if( loggingLevel(INFO) ) {
        Serial.printf_P(PSTR("                            Connection to %s failed with status %s\n"),ssid.c_str(),StatusStrings::wifiStatus());
      }
    }
  }
  else {
    if( loggingLevel(INFO) ) {
      String sid = WiFi.SSID();
      Serial.printf_P(PSTR("                            Connection to %s successful\n"),sid.c_str());
    }
  }
  return WiFi.status();
}

void WiFiPortal::finish() {
  _server.close();
  if( loggingLevel(FINE) ) Serial.printf_P(PSTR("WiFiPortal::finish: Internal Web Server closed\n"));  
  stopMDNS();
  if( loggingLevel(FINE) ) Serial.printf_P(PSTR("        mDNS stopped\n"));
     
  if( disconnectSoftAP() ) {
     WiFi.softAPdisconnect(true);
     if( loggingLevel(FINE) ) Serial.printf_P(PSTR("        SoftAP disconnected from %s\n"),_apName);
     WiFi.mode(WIFI_STA);
     if( loggingLevel(FINE) ) Serial.printf_P(PSTR("        WiFi reset to WIFI_STA mode\n"));
  }
  else {
     if( loggingLevel(FINE) ) Serial.printf_P(PSTR("        SoftAP NOT disconnected\n"));
  }
  delay(100);
  Serial.flush();
}

/*  Initialize WiFiPortal
 * 
 *   4. Start mDNS with apName
 *   3. Set WiFi Mode to WIFI_AP_STA and start the softAP
 *   5. Start an internal Web Server on port 8088
 *   6. Set up Request Handlers for the following pages: 
 *       /               - Main Portal page: Responds with a list of HTML Buttons (Styled with styles.css), one for each available SSID. 
 *                         Selecting one will bring up a form to enter Hostname, and PSK for the selected SSID
 *       /apForm        - Simple Web Form to enter Hoastname and PSK for a selected SSID
 *       /connect       - Expects a connection string on the query line: /connect?hostName=yourHostName&ssid=yourSSID&psk=yourPSK
 *                        Attempts a connection to ssid with the given psk, connection status is sent on finishConnect
 *       /finishConnect - Respons with status of last connection attempt
 *       /style.css     - Responds with CSS Styles for portal page
 *       /Notfound      - Responds with a simple OOPS! page
 *       
 */
void WiFiPortal::startPortal() {
    startMDNS();
    if( loggingLevel(FINE) ) Serial.printf_P(PSTR("WiFiPortal::startPortal: mDNS started on %s\n"),_apName);
    resetAP();
    
/**
 *  Setup Web handlers
 */
    _server.begin(SERVER_PORT);
    _ctx.setup(&_server,WiFi.softAPIP(),SERVER_PORT);
    WebContext* ctxPtr = &_ctx;
    _server.addHandler(new RequestLogger(this));
    _server.onNotFound([this]{
         Serial.printf("OnNotFound:  %s NOT FOUND\n",this->_server.uri().c_str());
         String content = String("<!DOCTYPE html><html><body style=\"font-family: Calibri\"><h1 align=\"center\"> OOPS! " + this->_server.uri() + " Not Found!</h1></body></html>");
         this->_server.send(200, "text/html", content);
      });
    _ctx.on("/",[this](WebContext* svr){this->display(svr);});
    _ctx.on("/connect",[this](WebContext* svr){this->connect(svr);});
    _ctx.on("/finishConnect",[this](WebContext* svr){this->finishConnect(svr);});
    _ctx.on("/styles.css",[ctxPtr](WebContext* ){ctxPtr->send_P(200, "text/css", styles_css);}); 
    _ctx.on("/apForm",[this](WebContext* svr){this->apForm(svr);});
    if( loggingLevel(FINE) ) Serial.printf_P(PSTR("WiFiPortal::startPortal: Internal Web Server started on %s:%d\n"),WiFi.softAPIP().toString().c_str(),SERVER_PORT);
}

void WiFiPortal::setup(const char* apName, const char* apPSK) {

/** Set up the soft AP and start mDNS on the SSID name
 *  
 */
 if( apName != NULL ) _apName = apName;
 if( apPSK != NULL )  _apPSK  = apPSK;
  
/**
 *   Attempt a Connection with cached credentials. If successful we're done, otherwise 
 *   set up the portal.
 */
  WiFi.mode(WIFI_STA);
  if( hasHostName() ) WiFi.setHostname(hostname());

  if( WiFi.getAutoConnect() ) Serial.printf_P(PSTR("WiFiPortal::setup: Autoconnect is true, attempt connection with stored credentials\n"));
  else Serial.printf_P(PSTR("WiFiPortal::setup: Autoconnect is false, Portal will be started directly\n"));

/**
 *  In general autoconnect should be set to true, unless resetCredentials() was called, in which case
 *  we start the portal as if the connection attempt failed. Autoconnect is then set to true. WiFi is
 *  always in persistent mode.
 */
  WiFi.persistent(true);
  if( WiFi.getAutoConnect() ) {
     WiFi.begin();
     if( WiFi.waitForConnectResult(cnxTimeout()) != WL_CONNECTED ) {
       setConnectionState(CNX_DISCONNECTED);
       startPortal();
       if(loggingLevel(WARNING)) {
         if(hasSSID()) Serial.printf_P(PSTR("WiFiPortal::setup: Connection to %s FAILED\n"),ssid());
         else Serial.printf_P(PSTR("WiFiPortal::setup: Connection FAILED, no SSID present\n"));
         Serial.printf("                   Set Access Point to %s with PSK %s and point a browser to %s.local (%s)\n",_apName,_apPSK,_apName,WiFi.softAPIP().toString().c_str());
       }
     }
     else {
/**
 *     Connection with stored cedentials was successful and at this point WiFi mode is WIFI_STA because the portal never
 *     actually started. If the softAP is not supposed to be disconnected, then set mode to WIFI_AP_STA and start up the softAP
 */
       setConnectionState(CNX_CONNECTED);
       if( !disconnectSoftAP() ) {
         Serial.printf("Setting mode to WIFI_AP_STA and starting softAP\n");
         WiFi.mode(WIFI_AP_STA);
         WiFi.softAP(_apName,_apPSK);
         Serial.printf("WiFi mode is %s\n",StatusStrings::wifiMode(WiFi.getMode()));
       }
     }       
  }
  else {
    setConnectionState(CNX_DISCONNECTED);
    startPortal();
    if(loggingLevel(INFO)) {
      Serial.printf_P(PSTR("WiFiPortal::setup: Starting Portal\n"));
      Serial.printf("                   Set Access Point to %s with PSK %s and point a browser to %s.local (%s)\n",_apName,_apPSK,_apName,WiFi.softAPIP().toString().c_str());
    }
  } 
}

void WiFiPortal::resetAP() {

  const char* title = "WiFiPortal::resetAP:";
  const char* tab   = "                    ";

  if( loggingLevel(FINE) ) Serial.printf_P(PSTR("%s Disconnecting from access point %s\n"),title,ssid());
  WiFi.disconnect();
  if( loggingLevel(FINE) ) Serial.printf_P(PSTR("%s Disconnecting Soft AP %s\n"),tab,_apName);
  WiFi.softAPdisconnect(true);

  if( WiFi.mode(WIFI_AP_STA) ) {
    if( loggingLevel(FINE) ) Serial.printf_P(PSTR("%s WiFi Mode set to WIFI_AP_STA\n"),tab);
  }
  else if( loggingLevel(FINE) ) Serial.printf_P(PSTR("%s Failed to set WiFi Mode to WIFI_AP_STA!\n"),tab);
  if( WiFi.softAP(_apName,_apPSK) ) {
     if( loggingLevel(FINE) ) Serial.printf_P(PSTR("%s Portal started with SSID %s and PSK %s\n"),tab,_apName,_apPSK);
     if( loggingLevel(FINE) ) Serial.printf_P(PSTR("%s Portal Access Point IP Address is %s\n"),tab,WiFi.softAPIP().toString().c_str());
  }
  else Serial.printf_P(PSTR("%s Portal FAILED to start Access Point %s!\n"),tab,_apName); 
}

/**
 *   Set WiFi autoconnect to false, so stored credentials will NOT be used on the next boot cycle.
 *   Must be called after WiFi.begin() to take effect.
 */
void WiFiPortal::resetCredentials() {
    WiFi.setAutoConnect(false);
}

/**
 *  Display the portal page consisting of buttons for each available Access Point
 */
void WiFiPortal::display(WebContext* svr) {
   char buffer[DISPLAY_SIZE];
   int size = sizeof(buffer);
   int pos = formatHeader(buffer,size,"Select An Access Point");
   WiFi.disconnect();
   WiFi.scanDelete();
   byte numSsid = WiFi.scanNetworks();
   if( loggingLevel(FINE) ) Serial.printf_P(PSTR("display: Number of SSIDs found is %d\n"),numSsid);
   for( int i=0; i<numSsid; i++ ) {
     String ssidStr = WiFi.SSID(i);
     const char* ssid = ssidStr.c_str();
     if(strlen(ssid) > 0) {
         if( loggingLevel(FINE) ) Serial.printf_P(PSTR("              Adding SSID %s to list\n"),ssid);
         pos = formatBuffer_P(buffer,size,pos,APwifi_button,"/apForm",ssid,ssid);
     }
   }
   formatTail(buffer,size,pos);
   svr->send(200, "text/html", buffer);
}

/**
 *  The form for entering PSK and optional hostName for the device
 */
void WiFiPortal::apForm(WebContext* svr) {
   char buffer[DISPLAY_SIZE];
   int size = sizeof(buffer);
   char title[100];
   String ssid = "ssid";
   int numArgs = svr->argCount();
   for( int i=0; i<numArgs; i++) {
      const String& argName = svr->argName(i);
      if( argName.equalsIgnoreCase("SSID") ) {ssid=svr->arg(i);}
   } 

/** 
 *  Form title
 */
   snprintf(title,100,"Enter PassKey for %s",ssid.c_str());
   int pos = formatHeader(buffer,size,title);

/**
 *  Form content. The form submit path is "/connect" and form cancel path is "/". Note that hash and salt are both empty strings here.
 */
   pos = formatBuffer_P(buffer,size,pos,AP_pskEntry,"/connect",ssid.c_str(),"/",ssid.c_str());

/**
 *  Form tail
 */
   formatTail(buffer,size,pos);
   svr->send(200,"text/html",buffer);
}

/**
 *  404 Not found page
 */
void WiFiPortal::notFound(WebContext* svr) {
  if( loggingLevel(FINE) ) Serial.printf_P(PSTR("OnNotFound:  %s NOT FOUND\n"),svr->getURI().c_str());
  char buffer[DISPLAY_SIZE];
  int size = sizeof(buffer);
  char title[100];
  snprintf(title,100,"OOPS! %s Not Found!",svr->getURI().c_str());
  int pos = formatHeader(buffer,size,title);
  formatTail(buffer,size,pos);
  svr->send(200,"text/html",buffer);
}

/**
 *   Pull Request args ssid and psk and attempt a connection to sssid with psk
 *   ESP8266 has quirkey behavior in that when a WiFi connection to an access point fails (with a bad psk) it closes
 *   any pending Web Server clients, so we send a response BEFORE attempting a connection. The response is an intermediate
 *   page (/finishConnect) which then finalizes everything.
 */
void WiFiPortal::connect(WebContext* svr) {
  char buffer[DISPLAY_SIZE];
  int size = sizeof(buffer);
  String ssid = "";
  String psk  = "";
  int numArgs = svr->argCount();
  if( numArgs > 1 ) {
      for( int i=0; i<numArgs; i++ ) {
         String argName = svr->argName(i);
         if( argName.equalsIgnoreCase("SSID" )) {ssid = svr->arg(i);ssid.trim();}
         else if( argName.equalsIgnoreCase("PSK" )) {psk = svr->arg(i);psk.trim();}
      }
      if( (ssid.length() > 0) && (psk.length() > 0) ) {
        
         if( loggingLevel(FINE) ) Serial.printf_P(PSTR("connect: Attempting connection to ssid = %s with psk = %s\n"),ssid.c_str(),psk.c_str()); 

 /**
  *  ESP8266 will randomly disconnect the Web Client during a connection attempt, particularly if the connection attempt fails, so 
  *  we send an intermediate page back now and make the user come back for a last connection attempt status.
  */
         formatBuffer_P(buffer,size,0,APfinish_connect,ssid.c_str(),"/finishConnect","/");
         svr->send(200,"text/html",buffer);
         if( loggingLevel(FINE) ) Serial.printf_P(PSTR("         Response sent\n"));    
/**
 *       Set credentials and attempt to connect
 *       Connection state is set to CNX_DISCONNECTED so connectWiFi() continues to handle web requests until complete. 
 *       The next page displayed will be finnishConnect(), WiFiConnector.lastStatus() will reflect WL_CONNECTED.
 */
         setConnectionState(CNX_DISCONNECTED);
         setSSID(ssid);
         if( attemptConnect(ssid, psk) == WL_CONNECTED ) {
             if( loggingLevel(FINE) ) Serial.printf_P(PSTR("connect: Connection attempt to %s SUCCESSFUL\n"),ssid.c_str());
/**
 *           At this point, WiFi.status() will be WL_CONNECTED but Portal state is CNX_DISCONNECTED. This will drive us back
 *           through connectWiFi to handle additional web requests until sequence is complete
 */
         }
         else {
             if( loggingLevel(FINE) ) Serial.printf_P(PSTR("connect: Connection attempt to %s FAILED!\n"),ssid.c_str());
/**
 *           At this point, WiFi.status() will NOT be WL_CONNECTED and Portal state will be CNX_DISCONNECTED. This will drive us back
 *           through connectWiFi to handle additional web requests until sequence is complete. 
 */
         }
      }  
      else {
        if( loggingLevel(WARNING) ) Serial.printf_P(PSTR("connect: Error on form input - argName[0] = %s and argName[1] = %s\n"),svr->argName(0).c_str(),svr->argName(1).c_str());
        int pos = formatHeader(buffer,size,"ERROR - Wrong arguments sent!");
        formatTail(buffer,size,pos);
        svr->send(200, "text/html", buffer);
        if( loggingLevel(FINE) ) Serial.printf_P(PSTR("connect: Response sent\n"));    
      }
  }
  else {
    if( loggingLevel(WARNING) ) Serial.printf_P(PSTR("connect: Called with insufficient arguments - argCount = %d\n"),numArgs);
    int pos = formatHeader(buffer,size,"ERROR - Insufficient number of arguments sent!");
    formatTail(buffer,size,pos);
    svr->send(200, "text/html", buffer);
    if( loggingLevel(FINE) ) Serial.printf_P(PSTR("connect: Response sent\n"));    
  }
}

/**
 *  Pulls last connect attempt status and displays appropriate page, either finish or retry.
 */
void WiFiPortal::finishConnect(WebContext* svr) {
   if( WiFi.status() == WL_CONNECTED ) {
     if( loggingLevel(FINE) ) Serial.printf_P(PSTR("finishConnect: Last Connection attempt to %s was SUCCESSFUL! Sending response\n"),ssid());
     svr->send_P(200, "text/html", AP_success);
     setConnectionState(CNX_FINISHED);
   }
   else {
     if( loggingLevel(FINE) ) Serial.printf_P(PSTR("finishConnect: Last Connection attempt to %s FAILED! Sending response\n"),ssid());         
     svr->send_P(200, "text/html", AP_retry);
     setConnectionState(CNX_DISCONNECTED);
   }
  if( loggingLevel(FINE) ) Serial.printf_P(PSTR("               Response sent\n")); 
}

/**
 * End of Portal Web handlers
 * 
 */

}  // End of namespace lsc
