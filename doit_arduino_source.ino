
//Conection libraries
#include <ESP8266WiFi.h> // 
#include <PubSubClient.h>
#include <WiFiManager.h> 

WiFiClient espClient;
PubSubClient client(espClient);
const char* mqtt_server = "192.168.0.5";

//#include <DNSServer.h>
//#include <ESP8266WebServer.h>



#include <string.h>
#include <ArduinoJson.h>


//DHT Sensor
#include <DHT.h>
#include <DHT_U.h>
#define DHTPIN 2 // D4
#define DHTTYPE DHT11
DHT_Unified dht(DHTPIN, DHTTYPE);


//MQ2 Sensor
#include <MQ2.h>
int Analog_Input = A0;
MQ2 mq2(Analog_Input);



//Servo Motor
#include <Servo.h>
Servo servoMotor;


//Magnetic Sensor
const int mageneticSensor1 = 12; //D6
int stateMageneticSensor1;

//Global Variables
uint32_t delayMS;
long lastMsg = 0;
char msg[50];
int value = 0;
const int led = 2;
int alarmOn = 0;



//Devices Setup

void wifimanager_setup(){
  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  
  // Uncomment and run it once, if you want to erase all the stored information
  // wifiManager.resetSettings();
  
  // set custom ip for portal
  //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  // fetches ssid and pass from eeprom and tries to connect
  // if it does not connect it starts an access point with the specified name
  // here  "AutoConnectAP"
  // and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("DiotNetSettings");
  // or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();
  
  // if you get here you have connected to the WiFi
  Serial.println("Connected.");
  }

  

char* mq2_json(){
  char lpg[100];
  char co[100];
  char smoke[100];
  char buffer[512];
  const int capacity=JSON_OBJECT_SIZE(6);
  StaticJsonDocument <capacity> doc;
  sprintf(lpg, "%f", mq2.readLPG());
  sprintf(co, "%f", mq2.readCO());
  Serial.println(mq2.readSmoke());

  sprintf(smoke, "%f", mq2.readSmoke());
  doc["lpg"] = lpg;
  doc["co"] = co;
  doc["smoke"] = smoke;
  serializeJson(doc, Serial);
  serializeJson(doc, buffer);
 
  return buffer;
  }
  

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

void pressButton() {
  servoMotor.write(180);
  delay(1000);  
  servoMotor.write(0);
  delay(1000);
  
  }

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println("");
  if (strcmp(topic,"home/door/")==0){
     Serial.print("Inside door");
    if ((char)payload[0] == 'H') {
      digitalWrite(led, LOW);   // Turn the LED on (Note that LOW is the voltage level
    }else if((char)payload[0] == 'L') {
      digitalWrite(led, HIGH);  // Turn the LED off by making the voltage HIGH
      Serial.print("Inside on");

      }
    }else if(strcmp(topic,"home/open/")==0){
      Serial.print("Incoming order press button");
      if ((char)payload[0] == '1') {
        digitalWrite(led, HIGH);  
        
        pressButton();
        delay(2000);
        digitalWrite(led, LOW);
        }
      }
     else if(strcmp(topic,"home/alarm_on/")==0){
      if ((char)payload[0] == '1') {
        alarmOn = 1;
        }
       else{
        alarmOn = 0;
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
      client.publish("home/temperature/", "1");
      client.publish("home/temperature/", "1");
      // Subscrioción de canales
      client.subscribe("home/door/");
      client.subscribe("home/open/");
      client.subscribe("home/alarm_on/");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//manual wifi setup
const char* ssid = "";
const char* password = "";
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

void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);     // Initialize the led pin as an output
  //setup_wifi(); //uncoment if you need a manual wifi setup
  wifimanager_setup();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  setup_dht11();
  servoMotor.attach(5);
  pinMode(mageneticSensor1, INPUT_PULLUP);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  Serial.print("Publish message: ");
  delay(500);
  client.publish("home/temperature/",dht11_());
  client.publish("home/smoke/",mq2_json());
  delay(1000);
  if (alarmOn == 1){
    Serial.println("get state");
    stateMageneticSensor1 = digitalRead(mageneticSensor1);
    if (stateMageneticSensor1 == HIGH){
      client.publish("home/alarm/","1");
      delay(500);
      }
    
    }
  
}
