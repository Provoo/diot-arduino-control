#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <DHT_U.h>
#include <string.h>

#include <ArduinoJson.h>

#define DHTPIN 0
#define DHTTYPE DHT11
DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;

    

// Update these with values suitable for your network.

const char* ssid = "Claro_TORRES";
const char* password = "1708342421";
const char* mqtt_server = "192.168.0.8";
const int led = 2;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;


void setup_dht11(){
  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  delayMS = sensor.min_delay / 1000;
  
  }

char* dht11_(){
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  char temperature[50];
  char humidity[50];
  char buffer[512];
  const int capacity=JSON_OBJECT_SIZE(4);
  StaticJsonDocument <capacity> doc;
  
  
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);
    Serial.println(F("°C"));
    sprintf(temperature, "%f", event.temperature);
    doc["temperature"] = temperature;
    
  }
  delay(3000);

  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    Serial.print(F("Humidity: "));
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));
    sprintf(humidity, "%f", event.relative_humidity);
    doc["humidity"] = humidity;
  }
  
  serializeJson(doc, Serial);
  serializeJson(doc, buffer);
  return buffer;
  }
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println(strcmp(topic,"home/door/"));
  if (strcmp(topic,"home/door/")==0){
     Serial.print("Inside door");
    if ((char)payload[0] == 'H') {
      digitalWrite(led, LOW);   // Turn the LED on (Note that LOW is the voltage level
      // but actually the LED is on; this is because
      Serial.print("Inside offf");
      // it is active low on the ESP-01)
    }else if((char)payload[0] == 'L') {
      digitalWrite(led, HIGH);  // Turn the LED off by making the voltage HIGH
      Serial.print("Inside on");

      }
    }

  // Switch on the LED if an 1 was received as first character
 
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("temperature_casa", "hello world");
      // Subscrioción de canales
      client.subscribe("home/door/");
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(led, OUTPUT);     // Initialize the led pin as an output
  
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  setup_dht11();
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  Serial.print("Publish message: ");
  client.publish("home/temperature/",dht11_());
  delay(1000);
  
}
