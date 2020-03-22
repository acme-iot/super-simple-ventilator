/* #include "webapi.h"

// Constructores
Webapi::Webapi(){

}

void Webapi::begin(){
    // attach class request handler
    classWebServer.on("/test", HTTP_ANY, std::bind(&Webapi::onRequestTest, this, std::placeholders::_1));
	classWebServer.onNotFound(std::bind(&Webapi::onRequestNotFound, this, std::placeholders::_1));
    classWebServer.begin();
}

void Webapi::onRequestNotFound(AsyncWebServerRequest *request){
    //Handle Unknown Request
    AsyncWebServerResponse *response = request->beginResponse(404); //Sends 404 File Not Found   
    response->addHeader("Access-Control-Allow-Origin","*");
    request->send(response);
}

void Webapi::onRequestTest(AsyncWebServerRequest *request){
    //Handle test Request
    AsyncJsonResponse * response = new AsyncJsonResponse();
    response->addHeader("Access-Control-Allow-Origin","*");
    request->send(response);
} */