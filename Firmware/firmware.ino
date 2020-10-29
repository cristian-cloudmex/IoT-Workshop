
//LIBRERIA DE WIFI
#include<ESP8266WiFi.h>
//libreria de mensajeria atravez del protocolo MQTT
#include<PubSubClient.h>
//manejo de archivos en formato JSON
#include<ArduinoJson.h>
#include<Arduino.h>
//Libreria para la manipulación del sensor HC-SCR4
#include<NewPing.h>

/*--------------------DATOS A CONFIGURAR----------------------------------*/

//Datos de conexion a internet en forma de punteros a char 

const char *ssid = "CloudMex";
const char *password = "U6DPXnP3tcSeDN5A"; 

/* Datos resultado del registro del dispositivo en 
IBM watson iot platform */ 

#define ORG_ID "97xmm2"
#define TYPE_ID "workshop"
#define DEVICE_ID "device1"
#define AUTH_TOKEN "hKHEj@qV@37te14?CM"

char server[] = ORG_ID ".messaging.internetofthings.ibmcloud.com";
char authMethod[] = "use-token-auth";
char token[] = AUTH_TOKEN;
char clientId[] = "d:" ORG_ID ":" TYPE_ID ":" DEVICE_ID ;

//topics de eventos y comandos para watson IoT platfomr
const char eventTopic[] = "iot-2/evt/status/fmt/json";
const char cmdTopic[] = "iot-2/cmd/led/fmt/json";
/*-------------------------------------------------------------------------*/

//Datos del sensor 
#define TRIGGER_PIN 16 //D0
#define ECHO_PIN 5  //D1
#define MAX_DISTANCE 200 //distancia maxima de sensado

//Objeto para controlar el sensor.
NewPing sonar(TRIGGER_PIN,ECHO_PIN,MAX_DISTANCE);

//Objeto manejador de conexion a wifi
WiFiClient wifiClient;

//Rutina cuando se manda un mensaje desde la consola de IBM IoT al dispositivo 
void callback(char* topic, byte* payload, unsigned int payloadLength) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < payloadLength; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if (payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}


//Creamos un objeto de conexionpara la plataforma de IBM IoT 
PubSubClient client(server, 1883, callback, wifiClient);

/* Intervalos de tiempo para enviar datos a la plataforma 
de IMB IoT */
int publishInterval = 5000; 
long lastPublishMillis;

void setup()
{
    /*realizamos la conexion serial, de wifi y con el dispositivo
    registrado en la  plataforma de IBM IoT */

    Serial.begin(115200); Serial.println();
    pinMode(LED_BUILTIN, OUTPUT);
    wifiConnect();
    mqttConnect();
    
}

void loop()
{
    if (millis() - lastPublishMillis > publishInterval) {
        publishData();
        lastPublishMillis = millis();
    }

    if (!client.loop()) {
        mqttConnect();
    }
}

//funciones necesarias 

/*------------------Conexion a red wifi---------------------------*/

void wifiConnect() {
  Serial.print("Connecting to "); Serial.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("nWiFi connected, IP address: "); Serial.println(WiFi.localIP());

}

/*-----------------------------------------------------------------------*/

/*------------------Conexion a la consola de IBM---------------------------*/

void mqttConnect() {
  if (!client.connected()) {
    Serial.print("Reconnecting MQTT client to "); Serial.println(server);
    while (!client.connect(clientId, authMethod, token)) {
      Serial.print(".");
      delay(500);
    }
    if (client.subscribe(cmdTopic)) {
      Serial.println("subscribe to responses OK");
    } else {
      Serial.println("subscribe to responses FAILED");
    }
    Serial.println();
  }
}

/*-----------------------------------------------------------------*/

/*-------------Envio de datos a la consola de IBM IoT-------------*/

void publishData() {

  delay(50);
  int sensorValue = sonar.ping_cm();
  Serial.print("Ping: ");
  Serial.print(sensorValue);
  Serial.println("cm"); 

  String payload = "{\"d\":{\"adc\":";
  payload += String(sensorValue, DEC);
  payload += "}}";

  Serial.print("Sending payload: "); Serial.println(payload);

  if (client.publish(eventTopic, (char*) payload.c_str())) {
    Serial.println("Publish OK");
  } else {
    Serial.println("Publish FAILED");
  }
}

/*-------------------------------------------------------------------------------*/
