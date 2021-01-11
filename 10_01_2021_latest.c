#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "NTPClient.h"
#include "WiFiUdp.h"
#include "FirebaseESP8266.h"
#include "DHT.h"

#define WLAN_SSID       "Snap!"
#define WLAN_PASS       "@Abhi98999"
#define FIREBASE_HOST   "snap-room-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH   "tirdyGd840uFZklAvNRhV722SUVEpt2MTf6KaOdS"
#define DHTTYPE DHT11 // DHT 11
#define DHTPIN 2 // D4

const int output1 = 5; //D1
const int output2 = 14;
const int output3 = 12;
const int output4 = 13;
const int pir_sig = 4; //D2
const long timeoutTime = 800;
const long utcOffsetInSeconds = 19800;

long duration;
String header;

unsigned long currentTime = millis();
unsigned long previousTime = 0;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

String output1State = "off";
String output2State = "off";
String output3State = "off";

WiFiServer server(80);
IPAddress local_IP(192, 168, 0, 164);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

WiFiClient client;
FirebaseData firebaseData;
DHT dht(DHTPIN, DHTTYPE);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

float getTemperature() {
  dht.begin();
  float temperature = dht.readTemperature();
  while (temperature != temperature ){
    temperature = dht.readTemperature();
  }
  return temperature;
}

float getHumidity() {
  dht.begin();
  float humidity = dht.readHumidity();
  while (humidity != humidity ){
    humidity = dht.readHumidity();
  }
  return humidity;
}

void setup() {
  pinMode(output1, OUTPUT);
  pinMode(output2, OUTPUT);
  pinMode(output3, OUTPUT);
  digitalWrite(output1, HIGH);
  digitalWrite(output2, HIGH);
  digitalWrite(output3, HIGH);

  Serial.begin(115200);

  Serial.print(F("Connecting to "));
  Serial.println(F(WLAN_SSID));
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println(F("STA Failed to configure"));
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }

  Serial.println(F(""));
  Serial.print(F("Connected with => "));
  Serial.print(WiFi.localIP());

  dht.begin();
  server.begin();
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  timeClient.begin();
  update_api_log(" -- Start", "Start");
  // Serial.println(getValue("1", "State"));
  setInitialState();
}

void setInitialState(){
  if (getValue("1", "State") == "true") {
    digitalWrite(output1, LOW);
    output1State = "on";
  }
  if (getValue("2", "State") == "true") {
    digitalWrite(output2, LOW);
    output1State = "on";
  }
  if (getValue("3", "State") == "true") {
    digitalWrite(output3, LOW);
    output1State = "on";
  }
}


void loop(){
  auto_restart();
  request_from_wifi();
}

void auto_restart(){
  if (int(ESP.getFreeHeap()) <= 30000){
    update_api_log(" -- Restart", "Restart");
    ESP.restart();
  }
}

void update_api_log(String api, String path){
  timeClient.update();
  Firebase.setString(firebaseData, path + "/" + daysOfTheWeek[timeClient.getDay()] + ", " + timeClient.getFormattedTime(), String(ESP.getFreeHeap()) + api);
}

void update_state(String device, String path, String value){
  timeClient.update();
  Firebase.setString(firebaseData, path + "/" + device, value);
}

String getValue(String device, String path){
  Firebase.getString(firebaseData, path + "/" + device);
  return firebaseData.stringData();
}

void request_from_wifi() {
  WiFiClient client = server.available();
  auto_restart();
  if (client) {                             // If a new client connects,
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          if (currentLine.length() == 0) {
            client.println(F("HTTP/1.1 200 OK"));
            client.println(F("Content-type:text/html"));
            client.println(F("Connection: close"));
            client.println();

            if (header.indexOf("GET /temperature") >= 0) {
              StaticJsonBuffer<200> jsonBuffer;
              JsonObject& root = jsonBuffer.createObject();
              root["temperature_sensor"] = getTemperature();
              root.printTo(client);
              update_api_log(" -- Temperature", "API");
            }
            else if (header.indexOf("GET /humidity") >= 0) {
              StaticJsonBuffer<200> jsonBuffer;
              JsonObject& root = jsonBuffer.createObject();
              root["humidity_sensor"] = getHumidity();
              root.printTo(client);
              update_api_log(" -- Humidity", "API");
            }
            else if (header.indexOf("GET /1/on") >= 0) {
              output1State = "on";
              digitalWrite(output1, LOW);
              update_api_log(" -- Shed Light/on", "API");
              update_state("1", "State", "true");
            }
            else if (header.indexOf("GET /1/off") >= 0) {
              output1State = "off";
              digitalWrite(output1, HIGH);
              update_api_log(" -- Shed Light/off", "API");
              update_state("1", "State", "false");
            }
            else if (header.indexOf("GET /2/on") >= 0) {
              output2State = "on";
              digitalWrite(output2, LOW);
              update_api_log(" -- Light/on", "API");
              update_state("2", "State", "true");
            }
            else if (header.indexOf("GET /2/off") >= 0) {
              output2State = "off";
              digitalWrite(output2, HIGH);
              update_api_log(" -- Light/off", "API");
              update_state("2", "State", "false");
            }
            else if (header.indexOf("GET /3/on") >= 0) {
              output3State = "on";
              digitalWrite(output3, LOW);
              update_api_log(" -- Fan/on", "API");
              update_state("3", "State", "true");
            }
            else if (header.indexOf("GET /3/off") >= 0) {
              output3State = "off";
              digitalWrite(output3, HIGH);
              update_api_log(" -- Fan/off", "API");
              update_state("4", "State", "false");
            }
            else if (header.indexOf("GET /restart") >= 0) {
              update_api_log(" -- Restart", "API");
              ESP.restart();
            }
            else if (header.indexOf("GET /on") >= 0) {
              output1State = "on";
              output2State = "on";
              output3State = "on";
              digitalWrite(output1, LOW);
              digitalWrite(output2, LOW);
              digitalWrite(output3, LOW);
              update_api_log(" -- All On ", "API");
            }
            else if (header.indexOf("GET /off") >= 0) {
              output1State = "off";
              output2State = "off";
              output3State = "off";
              digitalWrite(output1, HIGH);
              digitalWrite(output2, HIGH);
              digitalWrite(output3, HIGH);
              update_api_log(" -- All Off", "API");
            }
            else if (header.indexOf("GET /getState") >= 0) {
              StaticJsonBuffer<200> jsonBuffer;
              JsonObject& root = jsonBuffer.createObject();
              root["Shed Light"] = output1State;
              root["Light"] = output2State;
              root["Fan"] = output3State;
              root["Heap"] = String(ESP.getFreeHeap());
              root.printTo(client);
              update_api_log(" -- State", "API");
            }
          } else {
            currentLine = "";
          }
        }
        else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    header = "";
    client.stop();
  }
}
