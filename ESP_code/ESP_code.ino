#include "Arduino.h"
#include "string.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Adafruit_TCS34725.h"
#include "Wire.h"

//WiFi server
WiFiServer server(80);

//JSON documnets
StaticJsonDocument<512> doc;
StaticJsonDocument<512> doc_second;

// deviceId
uint8_t deviceId = 1;

// uveghaz id
String greenHouseId = "614f2d80351480b67fb8612d";

// rgb sensor piros
uint16_t colorRed = 0;

// rgb sensor zold
uint16_t colorGreen = 0;

// rgb sensor kek
uint16_t colorBlue = 0;

//Wifi server-re erkezett request
String header;

//Wifi halozat ssid
const char* ssid = "TEK megfigyelo allomas";

//Wifi halozat jelszo
const char* password = "0000000000";

//API vegpont eleresenek a "tove"
String apiRoute = "http://172.16.16.112:3000/api";

//Az UART-on keresztul kapott adat az STM mikrovezerlotol
String recivedDataFromSTM;

//RGB sensor
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_614MS, TCS34725_GAIN_1X);

//Csatlakozas a Wifi halozatra a megadott ssid es jelsza alapjan
void connectToWifiNetwork() {
  IPAddress local_IP(172, 16, 15, 46);
  IPAddress gateway(172, 16, 254, 254);
  IPAddress subnet(255, 255, 0, 0);
  WiFi.config(local_IP, gateway, subnet);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\n Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

//API GET hivas
String HTTPGetRequest(String additionalAPIRoute="null"){
   HTTPClient http;

   String payload = "";
   String apiRouteForRequest = apiRoute;
   //Ha erkezik parameterbe tovabbi route, akkor azt fuzzuk hozza
   if(additionalAPIRoute != "null")
   {
     apiRouteForRequest = apiRoute + additionalAPIRoute; 
   }

   Serial.println("ROUTE");
   Serial.println(apiRouteForRequest);
   http.begin(apiRouteForRequest);
   int httpResponseCode = http.GET();
   if (httpResponseCode>0)
   {
       Serial.print("HTTP Response code: ");
       Serial.println(httpResponseCode);
       payload = http.getString();
       Serial.println("PAYLOAD");
       Serial.println(payload);
   }
   else
   {
     Serial.print("Error code: ");
     Serial.println(httpResponseCode);
   }
   
   http.end();

   return payload;
}

//HTTP Post data
void HTTPPostData() {
  HTTPClient http;
  http.begin(apiRoute + "/device/measuredData");
  http.addHeader("Content-Type", "application/json");
  Serial.println(recivedDataFromSTM);
  http.POST(recivedDataFromSTM);
  http.end();
}

// RGB sensor ertek beolvasas
void measureColor() {
  uint16_t c;
  tcs.getRawData(&colorRed, &colorGreen, &colorBlue, &c);
}

//Inicializalas
void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 16, 17);
  connectToWifiNetwork();
  Wire1.begin(21, 22);
  
  if(tcs.begin(0x29, &Wire1))
  {
    Serial.println("Found sensor");
  } else {
    Serial.println("No sensor");
    while(1);
  }
}

void loop() {
  // API felol erkezett keresek kezelese
  WiFiClient client = server.available();
  if (client && client.available()) {
    while (client.connected()){
      char c = client.read();
      header += c;
      if (header.indexOf("GET /measure HTTP/1.1") >= 0) {
        Serial.println("START");
        char startChar[2] = {'0', '0'};
        Serial2.write(startChar);
        client.println("HTTP/1.1 200 OK");
        client.println("Content-type:text/html");
        client.println("Connection: close");
        client.println();
        header = "";
        client.stop();
      } 
      else if (header.indexOf("GET /config HTTP/1.1") >= 0)
      {
        String config = HTTPGetRequest("/config/deviceGetData?greenHouseId=" + greenHouseId + "&deviceId=" + deviceId);
        Serial.println("CONFIG");
        Serial.print(config);
        
        deserializeJson(doc_second, config);
        char configChar[2];
        int samplingTime = doc_second["config"];
        sprintf(configChar,"%d", samplingTime);
        Serial.println(configChar);
        Serial2.write(configChar);
        client.println("HTTP/1.1 200 OK");
        client.println("Content-type:text/html");
        client.println("Connection: close");
        client.println();
        header = "";
        client.stop();
      }
    }
  }
  
  if (Serial2.available() > 0) {
    recivedDataFromSTM = Serial2.readString();
    Serial.println(recivedDataFromSTM);

    // Ha 'S' karakter erkezik, akkor az a measurement_time inicializalas indulaskor
    if (recivedDataFromSTM[0] == 'S') 
    {
      String config = HTTPGetRequest("/config/deviceGetData?greenHouseId=" + greenHouseId + "&deviceId=" + deviceId);
        Serial.println("CONFIG");
        Serial.print(config);
        
        deserializeJson(doc_second, config);
        char configChar[2];
        int samplingTime = doc_second["config"];
        sprintf(configChar,"%d", samplingTime);
        Serial.println(configChar);
        Serial2.write(configChar);
        client.println("HTTP/1.1 200 OK");
        client.println("Content-type:tex  t/html");
        client.println("Connection: close");
        client.println();
        header = "";
        client.stop();
    } 
    else
    {
      measureColor();
      deserializeJson(doc, recivedDataFromSTM);
      doc["deviceId"] = deviceId;
      doc["greenHouseId"] = greenHouseId;
      doc["colorRed"] = colorRed;
      doc["colorGreen"] = colorGreen;
      doc["colorBlue"] = colorBlue;
      recivedDataFromSTM="";
      serializeJson(doc, recivedDataFromSTM);
      HTTPPostData();
    }
    Serial.println(recivedDataFromSTM);
  }
}
