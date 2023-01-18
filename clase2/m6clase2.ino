#include <TimeLib.h>

#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <ArduinoMqttClient.h>

#include <OneWire.h>                 //Se importan las librerías
#include <DallasTemperature.h>
#define Pin D5                        //Se declara el pin donde se conectará la DATA
OneWire ourWire(Pin);                //Se establece el pin declarado como bus para la comunicación OneWire
DallasTemperature sensors(&ourWire); //Se llama a la librería DallasTemperature

// Datos WiFi
char* ssid = "NOMBRE_RED_WIFI";
char* password = "PASSWORD";

// Datos MQTT
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
char* mqtt_user = "xxxxx";
char* mqtt_password = "";
const char broker[] = "test.mosquitto.org";
int        port     = 1883;
//int        port     = 8083;
const char topic[]  = "pucv/iot/m6/p1/g6";


void setup()
{
    Serial.begin(115200);
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

    //***** Configurando MQTT Client ************
    Serial.print("Attempting to connect to the MQTT broker: ");
    Serial.println(broker);
    //mqttClient.setUsernamePassword(mqtt_user,mqtt_password);
    
    if (!mqttClient.connect(broker, port))
    {
      Serial.print("MQTT connection failed! Error code = ");
      Serial.println(mqttClient.connectError());
      while(1);
    }

    Serial.println("You're connected to the MQTT broker!");
    Serial.println();
}

void loop()
{
    Serial.print("Requesting temperatures...");
    sensors.requestTemperatures();       //Prepara el sensor para la lectura
    Serial.println("DONE");
    float tempC = sensors.getTempCByIndex(0);
    
    // Check if reading was successful
    if (tempC != DEVICE_DISCONNECTED_C)
    {
        Serial.print("Temperature for the device 1 (index 0) is: ");
        Serial.println(tempC);
    }
    else
    {
        Serial.println("Error: Could not read temperature data");
    }

    // ******* Configuracion de JSON *********
    DynamicJsonDocument doc(1024);
    doc["temperature"] = tempC;
    String json;
    serializeJson(doc, json);

    Serial.print("Sending message to topic: ");
    Serial.println(topic);
    Serial.println(json);

    mqttClient.beginMessage(topic);
    mqttClient.print(json);
    mqttClient.endMessage();
  
    delay(15000);

}
