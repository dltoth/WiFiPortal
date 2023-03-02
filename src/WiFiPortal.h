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
 

#ifndef WIFI_PORTAL_H
#define WIFI_PORTAL_H

#include <Arduino.h>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#endif

#include <CommonProgmem.h>
#include <WebContext.h>

/** Leelanau Software Company namespace 
*  
*/
namespace lsc {
  

#define SERVER_PORT 80
#define CANCEL_SIZE 100
#define TIMEOUT     20000

typedef enum ConnectionState {
  CNX_DISCONNECTED,
  CNX_FINISHED,
  CNX_CONNECTED
} ConnectionState;

/** WiFiPortal provides a WiFi portal wrapper for either ESP8266 or ESP32. 
 *  At startup, the device attempts to connect with stored WiFi credentials, and if successful connectWiFi() returns immediately
 *  with CNX_CONNECTED. If unsuccessful, WiFiPortal will start up a captive portal interface to select an access point. Selecting an  
 *  access point then displays a form to enter the PSK, a connection attempt is made, and if sucessful, connectWiFi() returns  
 *  CNX_CONNECTED, completing the connection sequence. 
 *  Note:
 *    (1) WiFiPortal does not persist credentials in EEPROM, but rather relies on the underlying WiFi class for persistence. To override
 *        using stored credentials at startup, make the following calls prior to WiFiPortal::setup():
 *           WiFi.begin();
 *           WiFi.disconnect();
 *        This will erase stored credentials and the portal interface will start directly on WiFiPortal.setup(ssid,psk).
 *        Alternatively, WiFiPortal::resetCredentials() can be called after the connection sequence completes, and on the next boot
 *        cycle, no attempt will be made to use stored credentials; the portal interface will be started directly. This can be called
 *        from a Web page without disconnecting WiFi. WiFiPortal uses WiFi's underlying autoconnect flag to determine if stored 
 *        credentials should be used. Note that calling WiFi.setAutoconnect(true/false) will not take effect unless WiFi has been
 *        started (with WiFi.begin()).
 *    (2) WiFi (for both ESP8266 and ESP32) does not persist hostname, so applications using mDNS must code that directly into the device. 
 *        Setting hostname on WiFiPortal with WiFiPortal.setHostname(String) prior to WiFiPortal.setup() will pass hostname on to the 
 *        underlying WiFi, so the mDNS hostname and the router hostname will match.
 * 
 */
class WiFiPortal {
public:
  WiFiPortal() {}

  void             setup(const char* apName, const char* psk); 
  int              connectWiFi();

/**
 *   When WiFiPortal completes its connection sequence, the softAP can remain connected
 *   or it can be disconnected and put into WIFI_STA mode.
 */
  boolean          disconnectSoftAP()                      {return _disconnectSoftAP;}
  void             disconnectSoftAP(boolean flag)          {_disconnectSoftAP = flag;}
  const char*      apName()                                {return _apName;}
  const char*      apPsk()                                 {return _apPSK;}

  const char*      hostname()                              {return _hostname.c_str();}
  void             setHostname(String h)                   {_hostname = h;}
  void             setHostname(const char* host)           {String hostname(host); _hostname = hostname;}
  boolean          hasHostName()                           {return _hostname.length()!=0;}
  const char*      ssid()                                  {return _ssid.c_str();}
  boolean          hasSSID()                               {return _ssid.length()!=0;}
  int              cnxTimeout()                            {return _timeout;}
  void             cnxTimeout(unsigned long timeout)       {_timeout = timeout;}
  boolean          connectedState()                        {return _state == CNX_CONNECTED;}
  boolean          finishedState()                         {return _state == CNX_FINISHED;}
  boolean          disconnectedState()                     {return _state == CNX_DISCONNECTED;}
  ConnectionState  getConnectionState()                    {return _state;}

/**
 *  Reset Credentials. Portal is reset on next boot cycle of the device
 */
  static void      resetCredentials();

/**
 *  Set/Get/Check Logging Level. Logging Level can be NONE, INFO, FINE, and FINEST
 */
  void             logging(LoggingLevel level)             {_logging = level;}
  LoggingLevel     logging()                               {return _logging;}
  boolean          loggingLevel(LoggingLevel level)        {return(logging() >= level);}

  private:
  void             setSSID(String ssid)                    {_ssid = ssid;}
  void             setConnectionState(ConnectionState s)   {_state = s;}
  void             finish();
  void             startPortal();
  int              attemptConnect(String ssid, String psk);
/**
 *   Http handlers for the AP Portal. These methods are used when the device is acting as a portal
 *   in AP and STA mode. All handlers are set on the internal Web server
 */
  void             connect(WebContext* svr);           // Attempt a connection with SSID and PSK provided as server args
  void             finishConnect(WebContext* svr);     // Complete connection sequence
  void             display(WebContext* svr);           // Display portal page - a list of APs to select
  void             apForm(WebContext* svr);            // Display form to set PSK and hostName
  void             notFound(WebContext* svr);          // Displays 404 Not Found message for portal

/**
 *  mDNS abstration for ESP32 and ESP8266
 */
  bool             startMDNS();                        // Start MDNS if a hostname is present
  void             stopMDNS();                         // Stop MDNS
  void             updateMDNS();                       // Abstracted for ESP8266 and ESP32
  
  void             resetAP();                          // Reset soft AP state to start up

  boolean          _disconnectSoftAP  = true;
  const char*      _apName            = "WiFiPortal";
  const char*      _apPSK             = "admin";
  String           _ssid              = EMPTY_STRING;
  String           _hostname          = EMPTY_STRING; 
  unsigned long    _timeout           = TIMEOUT;
  MDNSResponder    _mDNS;
  WebContext       _ctx;
  LoggingLevel     _logging           = NONE;
  ConnectionState  _state             = CNX_DISCONNECTED;

#ifdef ESP8266
  ESP8266WebServer  _server;
#elif defined(ESP32)
  WebServer         _server;
#endif
  
  WiFiPortal(const WiFiPortal&)= delete;
  WiFiPortal& operator=(const WiFiPortal&)= delete;

};

class StatusStrings {
  public:
  StatusStrings() {}
  static const char* wifiMode(int mode) {
     switch(mode) {
         case WIFI_AP:
            return "WIFI_AP";
            break;
         case WIFI_STA:
            return "WIFI_STA";
            break;
         case WIFI_AP_STA:
            return "WIFI_AP_STA";
            break;
         default:
            return "WIFI_NONE";
            break;      
    }    
  }

  static const char*  encryptionType(int type) {
     switch( type ) {
       case 2:
       case 4:
          return "WPA";
       case 5:
          return "WEP";
       case 7:
          return "NONE";
       case 8:
          return "WPA";
       default:
          return "UNDEFINED";
       }
   }

  static const char* wifiStatus(int status) {
    const char* result = "WL_DISCONNECTED";
    switch(status) {
      case WL_CONNECTED:
         result = "WL_CONNECTED";
         break;
      case WL_NO_SHIELD:
         result = "WL_NO_SHIELD";
         break;
      case WL_IDLE_STATUS:
         result = "WL_IDLE_STATUS";
         break;
      case WL_NO_SSID_AVAIL:
         result = "WL_NO_SSID_AVAIL";
         break;
      case WL_SCAN_COMPLETED:
         result = "WL_SCAN_COMPLETED";
         break;
      case WL_CONNECT_FAILED:
         result = "WL_CONNECT_FAILED";
         break;
      case WL_CONNECTION_LOST:
         result = "WL_CONNECTION_LOST";
         break;
      case WL_DISCONNECTED:
         result = "WL_DISCONNECTED";
         break;
      }
    return result;
  }

static const char* wifiStatus() {return wifiStatus(WiFi.status());}
static const char* wifiMode()   {return wifiMode(WiFi.getMode());}
};

class RequestLogger : public RequestHandler {
  public:
  RequestLogger(WiFiPortal* portal) {_portal = portal;}

  bool canHandle(HTTPMethod method, String uri) {
    if( _portal->loggingLevel(FINE) ) {
      Serial.printf("RequestLogger: Portal Web Server procesing request for URI %s\n",uri.c_str());
      Serial.flush(); 
    }
    return false;
  }  

  private:
  WiFiPortal*     _portal = NULL;
  RequestLogger() {}
};

} // End of namespace lsc

#endif
