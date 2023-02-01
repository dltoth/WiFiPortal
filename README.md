# WiFiPortal
 WiFiPortal provides an HTML portal interface for selecting a local access point and entering a password. WiFi classes for both ESP8266 and ESP32 require  an application to hard code access point SSID and password, making it impossible to change without reflashing the device. WiFiPortal provides an easy to use HTML interface to select an access point and enter credentials. It uses the WiFi class itself to store credentials on the device, so the portal is only required when the device is in a new environment. WiFiPortal supports both ESP8266 and ESP32, but can be extended to other device architectures as well.
## Using WiFiPortal ##
Symantics for WiFiPortal are similar to that of the WiFi class itself:

```
portal.setup(SOFT_AP_SSID,SOFT_AP_PSK);
while(portal.connectWiFi() != CNX_CONNECTED) {delay(500);}
```
Setup() makes an initial connection attempt with stored credentials and if successful connectWiFi() will return immediately with a status of CNX_CONNECTED. If unsuccessful, it will start a captive portal and each iteration in the loop above will service HTTP requests until a successful connection is made.
WiFiPortal requires the additional [CommonUtil library](https://github.com/dltoth/CommonUtil/) for the HTML UI. <br>
### Example Sketch ###

```
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include "WiFiPortal.h"

using namespace lsc;


#define SOFT_AP_SSID "PortalSoftAP"
#define SOFT_AP_PSK  "hotSpot4"

#define          BOARD "ESP8266"
#define          HOSTNAME "BigBang"
WiFiPortal       portal;
 
void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  Serial.println();
  Serial.printf("Starting WiFiPortal example for board %s\n",BOARD);
  
/**
 *  Send progress to Serial. LoggingLevel can be NONE, WARNING, INFO, FINE, or FINEST
 */
  portal.logging(FINE);

/**
 *  Give Portal a hostname String to coordinate mDNS with the local router
 */
  portal.setHostname(HOSTNAME);

/**
 *  Initialize WiFiPortal with ssid and psk for the softAP, MUST be called prior to starting the connection 
 *  sequence.
 */
  portal.setup(SOFT_AP_SSID,SOFT_AP_PSK);
  
/** Start the connection sequence for WiFiPortal, if successful the portal will be discontinued and the application 
 *  will be connected to an Access Point with SSID and PSK input from the portal. When completed, WiFi is in 
 *  WIFI_STA mode and the softAP is disconnected. If a softAP is required for the application, set 
 *  disconnectSoftAP(false) prior to starting the connection sequence. 
 *   
 *   Connection sequence is similar to ESP8266/ESP32 WiFi. This loop will return once a valid SSID and PSK are 
 *   provided by either:
 *   1. Successfully connecting with previously stored credentials, or
 *   2. Successfull connection from credentials input from the portal interface, which will use the WiFi class to 
 *      persist for the next attempt
 *   Note the use of ConnectionState rather than WiFi status.
 */
  while(portal.connectWiFi() != CNX_CONNECTED) {delay(500);}
  
  Serial.printf("WiFi Connected to %s with IP address: %s\n",portal.ssid(),WiFi.localIP().toString().c_str());  

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
  if(portal.hasHostName()) MDNS.update();
}
```

### How It Works ###
 If the device is new to the local network, credentials will not have been persisted by the WiFi class, so WiFiPortal will start the soft AP with ssid "PortalSoftAP" and PSK "hotSpot4". To run this example:
 1. Open the Serial Monitor at 115200 to view program output and flash your device.
 2. Connect a laptop or mobile device to PortalSoftAP using hotSpot4 and point a browser to PortalSoftAP.local (192.168.4.1)
 3. Select your Access Point from the list
 4. Enter the PSK for your selected AP 
 5. Follow prompts to complete the connection sequence
 6. Your device is now connected to your access point and available at "BigBang.local"
 7. Restart your Arduino and the device should automatically re-connect to your access point.<br>

Note that the WiFi classes for ESP8266 and ESP32 do not persist hostname, so it must be hard coded into the application. Setting hostname with WiFiPortal will synchronize mDNS with the name that the WiFi class provides to your local router.
 

