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
 
#include <WiFiPortal.h>

using namespace lsc;


#define SOFT_AP_SSID "PortalHotSpot"
#define SOFT_AP_PSK  "hotSpot4"

#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#define           BOARD "ESP8266"
#elif defined(ESP32)
#include <WiFi.h>
#include <ESPmDNS.h>
#define           BOARD "ESP32"
#endif

const char*      hostname = "BigBang";
WiFiPortal       portal;

/** 
 *    Example use of WiFiPortal.
 *    If the device was last connected to a local access point, credentials will have been persisted on the device and
 *    WiFiPortal.connectWiFi() will return immediately with a value of CNX_CONNECTED.
 *    If the device has not been connected to a local access point before, credentials will not be persisted on the device,
 *    and the connector will start in WIFI_AP_STA mode with SSID = "PortalHotSpot" and PSK = "portaltest".
 *    0. Open the Serial Monitor at 115200 to view program output and flash your device.
 *    1. Connect a laptop or mobile device to PortalHotSpot using the above PSK and point a browser to 
 *       PortalHotSpot.local (192.168.4.1)
 *    2. Select your Access Point from the list
 *    3. Enter the PSK for your selected AP 
 *    4. Follow prompts to complete the connection sequence
 *    5. On the next boot cycle of the device, stored credentials will be used to connect to the same access point, if available.
 *    
 */
 
void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  Serial.println();
  Serial.printf("Starting BasicUsage example for board %s\n",BOARD);

/**
 *   Uncomment to erase stored credentials and force portal startup
 * Serial.printf("Disconnecting WiFi\n");
 * WiFi.begin();
 * WiFi.disconnect();
 */

/**
 *   Uncomment to keep the softAP running after connection sequence completes
 * portal.disconnectSoftAP(false);
 */

/**
 *  Send progress to Serial. LoggingLevel can be NONE, INFO, FINE, or FINEST. Must be called prior to setup().
 */
  portal.logging(FINE);

/**
 *  Give Portal a hostname String to coordinate mDNS with the local router, must be called prior to setup()
 */
  portal.setHostname(hostname);

/**
 *   Initialize WiFiPortal with ssid and psk for the softAP, MUST be called prior to starting the connection sequence.
 */
  portal.setup(SOFT_AP_SSID,SOFT_AP_PSK);  

/**  Start WiFi with AP Portal, if successful the AP Portal will be discontinued and the application will be connected
 *   to an Access Point with SSID and PSK input from the portal. When completed, WiFi is in WIFI_STA mode and the softAP
 *   is disconnected. If a softAP is required for the application, set disconnectSoftAP(false) prior to starting the
 *   connection sequence. 
 *   
 *   Connection sequence is similar to ESP8266/ESP32 WiFi. This loop will return once a valid SSID and PSK are provided by either:
 *   1. Reading successful SSID and PSK previously stored on the device or
 *   2. Successfully input from the portal interface. The WiFi class will persist credentials for the next use
 *   Note the use of ConnectionState rather than WiFi status.
 */
  while(portal.connectWiFi() != CNX_CONNECTED) {delay(500);}
  
  Serial.printf("WiFi Connected to %s with IP address: %s\n",portal.ssid(),WiFi.localIP().toString().c_str());  

/**
 *  Uncomment to start portal directly on the next boot cycle, without using stored credentials.
 *  This call does not disconnect from the access point so is suitable to call from a Web page. In the case
 *  below, be sure to re-comment these lines or stored credentials will never be used.
 * 
 * WiFiPortal::resetCredentials();
 * if( WiFi.getAutoConnect() ) Serial.printf("Autoconnect set to true, stored WiFi credentials WILL be used on the next boot cycle\n");
 * else  Serial.printf("Autoconnect set to false, stored WiFi credentials will NOT be used on the next boot cycle\n");
 */

/** 
 *  The portal interface allows for hostname input, if provided start mDNS.
 */
  if(portal.hasHostName()) {
      Serial.printf("Starting mDNS on %s\n",portal.hostname());
      MDNS.begin(portal.hostname());
  }
  else Serial.printf("Device hostname not provided\n");
  
}

void loop() {
#ifdef ESP8266
   if(portal.hasHostName()) MDNS.update();
#endif
}
