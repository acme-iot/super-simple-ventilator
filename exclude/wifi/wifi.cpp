#include "wifi.h"

// Constructores
WifiO::WifiO(){

}

WifiO::WifiO(IPAddress ip, IPAddress netmask){
  this->setApNetwork(ip,netmask);
}

int WifiO::connectWifiClient(){
  Serial.println("Connecting as wifi client...");
  WiFi.disconnect();
  WiFi.begin ( ssid, password );
  int connRes = WiFi.waitForConnectResult();
  switch(connRes) {
    case WL_CONNECTED:
      connectWlan = false;
      Serial.println("Connected as wifi client...");
      break;
    case WL_NO_SSID_AVAIL:
      Serial.println("SSID cannot be reached...");
      break;
    case WL_CONNECT_FAILED:
      Serial.println("Password is incorrect...");
      break;
    case WL_IDLE_STATUS:
      Serial.println("Idle Satus...");
      break;
    case WL_DISCONNECTED:
      Serial.println("module is not configured in station mode...");
      break;
  }
  lastConnectTry = millis();
  return connRes;
}

int WifiO::connectWifiAp(){
  Serial.print("Configuring access point...");
  WiFi.softAPConfig(apIP, apIP, apnetMsk);
  bool conectedAp = WiFi.softAP(softAP_ssid, softAP_password);
  Serial.println(conectedAp ? "Success" : "Failed");
  return conectedAp;
}

void WifiO::setAPCredentials(String _ssid, String _password){
  _ssid.toCharArray(softAP_ssid, sizeof(softAP_ssid) - 1);
  _password.toCharArray(softAP_password, sizeof(softAP_password) - 1);
}

void  WifiO::setApNetwork(IPAddress ip, IPAddress netmask){
   this->apIP     =  ip;
   this->apnetMsk   = netmask;
}

void WifiO::setStaNetwork(IPAddress ip, IPAddress netmask,IPAddress gateway,IPAddress dns0,IPAddress dns1){
  this->IP = ip;
  this->netmask = netmask;
  this->gateway = gateway;
  this->dns0 = dns0;
  this->dns1 = dns1;
}

void WifiO::shouldConnect(){
  connectWlan = strlen(ssid) > 0; // Request WLAN connect if there is a SSID
}

void WifiO::checkConnect(){
  if (connectWlan) {
    Serial.println ( "Connect requested" );
    connectWifiClient();
  }
}

void WifiO::wifiStatus(){
  int current = WiFi.status();
  if (current == 0 && millis() > (lastConnectTry + 60000) ) {
    Serial.print ( "Status: " );
    Serial.println ( current );
    /* If WLAN disconnected and idle try to connect */
    /* Don't set retry time too low as retry interfere the softAP operation */
    connectWlan = true;
  }
  if (status != current) { // WLAN status change
    Serial.print ( "Status: " );
    Serial.println ( current );
    status = current;
    if (current == WL_CONNECTED) {
      /* Just connected to WLAN */
      Serial.println ( "" );
      Serial.print ( "Connected to " );
      Serial.println ( ssid );
      Serial.print ( "IP address: " );
      Serial.println ( WiFi.localIP() );

      // Setup MDNS responder
      if (!MDNS.begin(myHostname)) {
        Serial.println("Error setting up MDNS responder!");
      } else {
        Serial.println("mDNS responder started");
        // Add service to MDNS-SD
        MDNS.addService("http", "tcp", 80);
      }
    } else if (current == WL_NO_SSID_AVAIL) {
      WiFi.disconnect();
    }
  }
}

void WifiO::setCredentials(String _ssid, String _password){
  _ssid.toCharArray(ssid, sizeof(ssid) - 1);
  _password.toCharArray(password, sizeof(password) - 1);
}

 void WifiO::scanNetworks(){
    //First request will return 0 results unless you start scan from somewhere else (loop/setup)
    WiFi.scanNetworks();
}