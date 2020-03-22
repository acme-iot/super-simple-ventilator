/*
*/
#ifndef Wifio_h
#define Wifio_h

#include <Arduino.h>

#include <WiFi.h>
#include <WiFiType.h>
//#include <ESP8266mDNS.h>

#include <WiFiClient.h>
#include <DNSServer.h>
#include <ArduinoJson.h>

class WifiO
{
  public:
    WifiO();
    WifiO(IPAddress ip, IPAddress netmask);

    void setApNetwork(IPAddress ip, IPAddress netmask);
    void setStaNetwork(IPAddress ip, IPAddress netmask,IPAddress gateway,IPAddress dns0,IPAddress dns1);

    void setCredentials(String _ssid, String _password);
    void setAPCredentials(String _ssid, String _password);

    int connectWifiClient();
    int connectWifiAp();

    void checkConnect();
    void shouldConnect();
    void wifiStatus();
    void scanNetworks();

  private:
    char myHostname[16];      /*hostname for mDNS. */
    DNSServer dnsServer;
    const byte DNS_PORT = 53;

    /* Soft AP Setting */
    char softAP_ssid[32];
    char softAP_password[32];
    IPAddress apIP ;
    IPAddress apnetMsk;

    /* Wifi Client Settings*/
    char ssid[32];
    char password[32];
    IPAddress IP;
    IPAddress netmask;
    IPAddress gateway;
    IPAddress dns0;
    IPAddress dns1;

    bool connectWlan;               /* Connect to WLAN asap? */
    long lastConnectTry = 0;        /* Last time I tried to connect to WLAN */
    int status = WL_IDLE_STATUS;    /* Current WLAN status */

};

#endif