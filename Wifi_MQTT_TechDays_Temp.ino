#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ESP8266WebServer.h>
#include <IRremoteESP8266.h>
#include <DHT.h>

const char* idESP8266= "ESP8266Client2";

/**
 * Flags Pins
 */
const boolean flagTEMP = true;
/**
 * Configurações de Pins
 */
/*const int pinTemp = 5;*/


/* 
 * Configurações da Temperatura 
 */
#define DHTPIN  2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE, 11);

/**
 * Configurações de Rede
 */
const char* ssid = "agricultura";
const char* password = "Password!23";

//IPAddress local_ip(192, 168, 0, 11);
//IPAddress gateway(192, 168, 0, 1);
//IPAddress subnet(255, 255, 255, 0);

/**
 * Configurações de MQTT
 */
const char* mqtt_server = "192.168.0.10";
WiFiClient espClient;
PubSubClient client(espClient);
char msg[50];

/**
 * Tempos de Atualização
 */
long lastTemp = 0;
long lastHum = 0;


/**
 * Chars Result
 */
char resultTemp[200];

/**
 * Setup
 */
void setup() {
  delay(1000);
  Serial.begin(115200);
  
  Serial.print("Configuring access point...");
  /*if(flagTEMP){
    dht.setup(DHTPIN);
  }*/
  
  setup_WIFI();
  setup_MQTT();
}

/**
 * Loop
 */
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if(flagTEMP){
    mqttTemp(resultTemp);
  }
}

/************************************ WIFI ************************************/

/**
 * Setup WIFI
 */
void setup_WIFI() {
  delay(10);
  
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  
  //WiFi.config(local_ip, gateway, subnet);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  IPAddress myIP = WiFi.localIP();

  
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(myIP);
}

float hum;
float temp;

/**
 * Temp
 * @param      {char*}   result
 */
void Temp(char* result) {  
  hum = dht.readHumidity();
  temp = dht.readTemperature();  
  
  snprintf(result, 200, "{\"hum\": %d.%d, \"temp\": %d.%d}", (int)hum, (int)((hum - (int)hum) * 100), (int)temp, (int)((temp - (int)temp) * 100));
}


/************************************ MQTT ************************************/
/**
 * Setup MQTT
 */
void setup_MQTT() {
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

/**
 * Callback MQTT
 */
void callback(char* top, byte* payload, unsigned int length) {
  String topic = top;
  String message;
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {    
    message += (char)payload[i];   
  }
}

/**
 * Reconnect
 */
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(idESP8266)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

/**
 * mqttTemp
 */
void mqttTemp(char* result) {
  long now = millis();
  if (now - lastTemp > 10000) {
    lastTemp = now;
    Temp(result); 
    client.publish("ESP8266_Temp", result);
  }
}
