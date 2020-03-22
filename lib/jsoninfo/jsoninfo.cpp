/* #include "jsonInfo.h"

void JsonInfo::ScanWifiJson(JsonObject &json){
  int n = WiFi.scanComplete();
  if(n == -2){
    WiFi.scanNetworks();
  } else if(n){
    JsonArray&  nestedArray  = json.createNestedArray("wifis");
    for (int i = 0; i < n; ++i){
      JsonObject& nestedObject = nestedArray.createNestedObject();
      nestedObject["rssi"]    = String(WiFi.RSSI(i));
      nestedObject["ssid"]    = String(WiFi.SSID(i)) ;
      nestedObject["channel"] = WiFi.channel(i);
      nestedObject["secure"]  = WiFi.encryptionType(i) == ENC_TYPE_NONE ? "false" : "true";
      nestedObject["hidden"]  = WiFi.isHidden(i) ? "true" : "false";
    }
    WiFi.scanDelete();
    if(WiFi.scanComplete() == -2){
      WiFi.scanNetworks(true);
    }
  }
}

void JsonInfo::InfoWifiAP(JsonObject &json){
   JsonObject& nestedObject   = json.createNestedObject("SAP");
   nestedObject["macAddress"] = String(WiFi.softAPmacAddress());
   nestedObject["stations"]   = WiFi.softAPgetStationNum();
   nestedObject["ip"]         = StringIpAddress(WiFi.softAPIP()) ;
}

void JsonInfo::InfoWifiSTA(JsonObject &json){
   JsonObject& nestedObject = json.createNestedObject("STA");
   int statusWifi = WiFi.status();
   nestedObject["wifiStatus"] = statusWifi;
   nestedObject["macAddress"] = String(WiFi.macAddress());
   if( WL_CONNECTED  == statusWifi)
   {
     InfoSTA(json);
     InfoNetworkSTA(json);
   }
 }

void JsonInfo::InfoSTA(JsonObject &json){
   JsonObject& nestedObject = json.createNestedObject("InfoAp");
   nestedObject["ssid"]     = WiFi.SSID();
   nestedObject["rssi"]     = WiFi.RSSI();
}

void JsonInfo::InfoNetworkSTA(JsonObject &json){
   JsonObject& nestedObject = json.createNestedObject("Network");
   nestedObject["ip"]       = StringIpAddress(WiFi.localIP()) ;
   nestedObject["submask"]  = StringIpAddress(WiFi.subnetMask()) ;
   nestedObject["gateway"]  = StringIpAddress(WiFi.gatewayIP()) ;
   nestedObject["dns0"]     = StringIpAddress(WiFi.dnsIP(0)) ;
   nestedObject["dns1"]     = StringIpAddress(WiFi.dnsIP(1)) ;
   nestedObject["hostname"] = WiFi.hostname();
}

String JsonInfo::StringIpAddress(IPAddress address){
   return String(address[0]) + "." +
          String(address[1]) + "." +
          String(address[2]) + "." +
          String(address[3]);
}
 */