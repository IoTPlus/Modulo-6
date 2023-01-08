#include <TimeLib.h>
#include <ESP8266WiFi.h>        // WiFi 1.2.7
#include <ESP8266HTTPClient.h>  // WiFi 1.2.7
#include <ArduinoJson.h>        //ArduinoJson 6.19.14
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <OneWire.h>                 //Se importan las librerías
#include <DallasTemperature.h>

float temp; 

OneWire bus(D5);
DallasTemperature sensor_temp(&bus); 

// Datos WiFi
char* ssid = ""; 
char* password = "";

char* url = "https://industrial.api.ubidots.com/api/v1.6/devices/georgedevice"; 
//char* url = "https://industrial.api.ubidots.com/api/v1.6/devices/patriciodevice";
//char* url = "https://industrial.api.ubidots.com/api/v1.6/devices/matiasdevice";

char* ntpServer = "ntp.shoa.cl"; 
//long gmtOffset_sec = -10800; //For local time
long gmtOffset_sec = 0; //For UTC time

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "ntp.shoa.cl");

unsigned long epochtime;
String formattedDate;
String dayStamp;
String timeStamp;

void setup() {
  sensor_temp.begin();
  Serial.begin(9600); 
  //connect to WiFi 
  Serial.print("Connecting to "); 
  Serial.println(ssid); 
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); 
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print("."); 
    } 
  Serial.print("CONNECTED WITH LOCAL IP ADDRESS: ");
  Serial.println(WiFi.localIP());
  timeClient.begin();
  timeClient.setTimeOffset(gmtOffset_sec);
  } 

void loop() { 
  //while(!timeClient.update()){
  //  timeClient.forceUpdate();
  //  }
  //formattedDate = timeClient.getFormattedTime();
  timeClient.update();
  epochtime = timeClient.getEpochTime();
  //This is another way to get epoch time in ms, by "casting" (means changing the type of the variable that is returned by a function), this allows us to get the right timestamp in epoch format
  //unsigned long long epochtime = (unsigned long long) timeClient.getEpochTime() * 1000;
  sensor_temp.requestTemperatures();      //Prepara el sensor para la lectura
  //Serial.println("DONE");
  temp = sensor_temp.getTempCByIndex(0);
  Serial.print(epochtime);
  Serial.print("    ");
  printf("Date: %4d-%02d-%02d %02d:%02d:%02d\n", year(epochtime), month(epochtime), day(epochtime), hour(epochtime), minute(epochtime), second(epochtime));
  Serial.print("    ");
  Serial.println(temp);
  
  //Subiendo la lectura del tiempo al mock server:
  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure(); //the magic line, use with caution
  
  if(http.begin(client, url)) //Iniciar conexión
  {
      Serial.print("[HTTP] POST...\n");

      // ******* Configuracion de JSON *********
      DynamicJsonDocument doc(1024);
      DynamicJsonDocument doc_george(1024);
      doc_george["value"]=temp;
      doc_george["timestamp"]=epochtime;
      //doc["context"]={"name":"George", "status":"Success"};
      String json_george;
      serializeJson(doc_george, json_george);
      doc["Temperature"]=json_george;
      String json;
      serializeJson(doc, json);
      http.addHeader("X-Auth-Token", "BBFF-lA0XGKoSBDQxufJgUp5NkN1KWQBVbz"); //Go to Settings, then to API Credentials, then to Default Token, paste it here
      
      int httpResponseCode = http.POST(json);
      if(httpResponseCode > 0)
      {
          Serial.printf("[HTTP] POST... code: %d\n", httpResponseCode);
  
          if (httpResponseCode == HTTP_CODE_OK || httpResponseCode == HTTP_CODE_MOVED_PERMANENTLY)
          {
              String payload = http.getString();   // Obtener respuesta
              Serial.println(payload);   // Mostrar respuesta por serial
          }
      }
      else
      {
          Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
      }
  
      http.end();
  }
  else
  {
      Serial.printf("[HTTP} Unable to connect\n");
  }
  delay(5000); 
}
