# WiFiPortal
 WiFiPortal provides an HTML portal interface for selecting a local access point and entering a password. WiFi classes for both ESP8266 and ESP32 require  an application to hard code access point SSID and password, making it impossible to change without reflashing the device. WiFiPortal provides an easy to use HTML interface to select an access point and enter credentials. It uses the WiFi class itself to store credentials on the device, so the portal is only required when the device is in a new environment. WiFiPortal supports both ESP8266 and ESP32, but can be extended to other device architectures as well.
 
## Using WiFiPortal ##

Semantics for WiFiPortal are similar to that of the WiFi class itself:

```
portal.setup(SOFT_AP_SSID,SOFT_AP_PSK);
while(portal.connectWiFi() != CNX_CONNECTED) {delay(500);}
```

Setup() makes an initial connection attempt with stored credentials and if successful connectWiFi() will return immediately with a status of CNX_CONNECTED. If unsuccessful, it will start a captive portal and each iteration in the loop above will service HTTP requests until a successful connection is made.
WiFiPortal requires the additional [CommonUtil library](https://github.com/dltoth/CommonUtil/) for the HTML UI.

### Example Sketch ###

An example sketch with WiFiPortal can be found in [examples/WiFiPortal](). The important parts are:

**Namespace Declaration**

The namespace *lsc* is used for WiFiPortal and all other libraries [ssdp](https://github.com/dltoth/ssdp/), [UPnPDevice](https://github.com/dltoth/UPnPDevice/), [DeviceLib](https://github.com/dltoth/DeviceLib/), and [CommonUtil](https://github.com/dltoth/CommonUtil/). Also, the captive portal runs on an access point provided by the WiFi class, for both ESP32 and ESP8266; SSID and PSK are provided.

```
using namespace lsc;

#define SOFT_AP_SSID "PortalHotSpot"
#define SOFT_AP_PSK  "hotSpot4"

```
**Portal Setup**

Logging messages can be sent to the Serial port by setting the logging level to NONE, WARNING, INFO, FINE, or FINEST. If a *hostname* is set, then the WiFi class will use *hostname* to register with the local router, so mDNS and local router will be consistent. Lastly, set the Soft Access Point ssid and psk. The setup() method also attempts a connection with credentials persisted by the WiFi class (if they exist).

```
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

```
**Start the Connection Sequence**

Semantics for WiFiPortal are similar to that of the WiFi class; loop until a successful connection is made. If credentials were persisted by the WiFi class, and the connection attempt in setup() was successful, connectWiFi() will return immediately with CNX_CONNECTED. Otherwise, a captive portal will be started and each iteration of the loop below will service HTTP requests until ssid and psk are provided and connection is successful.

```
  while(portal.connectWiFi() != CNX_CONNECTED) {delay(500);}

```

### How It Works ###

 If the device is new to the local network, credentials will not have been persisted by the WiFi class, so WiFiPortal will start the soft AP with ssid *PortalSoftAP* and PSK *hotSpot4*. To run this example:
 1. Open the Serial Monitor at 115200 to view program output and flash your device.
 2. Connect a laptop or mobile device to *PortalSoftAP* using *hotSpot4* and point a browser to *PortalSoftAP.local* (192.168.4.1)
 3. Select your Access Point from the list
 4. Enter the PSK for your selected AP 
 5. Follow prompts to complete the connection sequence
 6. Disconnect your laptop or mobile device from *PortalSoftAP* and re-connect to your accesspoint. Your ESP device is now available on your local network at "BigBang.local"
 7. Restart your ESP and the device should automatically re-connect to your access point.

Note that the WiFi classes for ESP8266 and ESP32 do not persist hostname, so it must be hard coded into the application. Setting hostname with WiFiPortal will synchronize mDNS with the name that the WiFi class provides to your local router.
 

