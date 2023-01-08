#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <ArduinoMqttClient.h>
#include <TimeLib.h>
#include <OneWire.h>                 //Se importan las librerías
#include <DallasTemperature.h>

float temp; 
int adc_moisture;
float moisture;
int moist;
/*
@100% y = 0
@0% y = 1023

y=mx+b
b=1023
-1023=mx
m=-10.23
y=(-10.23)x+1023
*/

OneWire bus(D5);
DallasTemperature sensor_temp(&bus); 

// Datos WiFi

char* ssid = ""; 
char* password = ""; 

char* ntpServer = "ntp.shoa.cl"; 
//long gmtOffset_sec = -10800; //For local time
long gmtOffset_sec = 0; //For UTC

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "ntp.shoa.cl");

// Datos MQTT
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

//char* mqtt_user = "xxxxx";
//char* mqtt_password = "";
const char* mqttServer = "test.mosquitto.org";
int port = 1883;
//const char* subscribeTopic  = "pucv/iot/m6/p1/George_Saavedra";  //My topic
const char* subscribeTopic  = "pucv/iot/m6/p1";   //Team topic

unsigned long epochtime;

void setup()
{
    sensor_temp.begin();
    Serial.begin(115200);
    //Serial.setDebugOutput(true);
    delay(500);
    
    Serial.print("Connecting to WiFi ");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.print("Connected");

    //***** Obtener direccion IP asignada *******
    IPAddress ip;
    ip = WiFi.localIP();
    Serial.print("Direccion IP: ");
    Serial.println(ip);

    // Initialize a NTPClient to get time
    timeClient.begin();
    timeClient.setTimeOffset(gmtOffset_sec);

    //***** Configurando MQTT Client ************
    Serial.print("Attempting to connect to the MQTT broker: ");
    Serial.println(mqttServer);
    //mqttClient.setUsernamePassword(mqtt_user,mqtt_password);
    
    if (!mqttClient.connect(mqttServer, port))
    {
      Serial.print("MQTT connection failed! Error code = ");
      Serial.println(mqttClient.connectError());
      while(1);
    }

    Serial.println("You're connected to the MQTT broker!");
    Serial.println();
}

void loop(){
  //while(!timeClient.update()) {
  //  timeClient.forceUpdate();
  //}
  timeClient.update();
  epochtime = timeClient.getEpochTime();
  Serial.print("Epoch Time: ");
  Serial.print(epochtime);
  Serial.print("    ");
  printf("Date: %4d-%02d-%02d %02d:%02d:%02d\n", year(epochtime), month(epochtime), day(epochtime), hour(epochtime), minute(epochtime), second(epochtime));

  //Serial.print("Requesting temperatures...");
  sensor_temp.requestTemperatures();      //Prepara el sensor para la lectura
  //Serial.println("DONE");
  temp = sensor_temp.getTempCByIndex(0);
  // Check if reading was successful
  if (temp != DEVICE_DISCONNECTED_C){
    Serial.print("Temperature for the device 1 (index 0) is: ");
    Serial.println(temp);
    }
  else{
    Serial.println("Error: Could not read temperature data");
    }
  adc_moisture = analogRead(A0);
  moisture = (adc_moisture-1023)/(-10.23);  
  moist=round(moisture);
  Serial.print("Plant moisture:    ");
  Serial.print(moist);
  Serial.println(" %");
  // ******* Configuracion de JSON *********
  DynamicJsonDocument doc(1024);
  doc["device_id"]="wemos_george";
  doc["ts"]=epochtime;
  doc["temperature"]=temp;
  doc["moisture"]=moist;

  String json;
  serializeJson(doc, json);

  Serial.print("Sending message to topic: ");
  Serial.println(subscribeTopic);
  Serial.println(json);

  mqttClient.beginMessage(subscribeTopic);
  mqttClient.print(json);
  mqttClient.endMessage();

  delay(15000);
}
