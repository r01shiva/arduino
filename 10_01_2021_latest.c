#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "DHT.h"

#define WLAN_SSID       "Snap!"
#define WLAN_PASS       "@Abhi98999"
#define FIREBASE_HOST   "snap-room-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH   "tirdyGd840uFZklAvNRhV722SUVEpt2MTf6KaOdS"

FirebaseData firebaseData1;
FirebaseData firebaseData2;

#define DHTPIN 2 // D4
const int output1 = 5; //D1
const int output2 = 14;
const int output3 = 12;
const int output4 = 13;
const int pir_sig = 4; //D2
const long timeoutTime = 2000;

#define DHTTYPE DHT11 // DHT 11
String header;
long duration;
unsigned long currentTime = millis();
unsigned long previousTime = 0;

String output1State = "off";
String output2State = "off";
String output3State = "off";
String pirState = "off";

WiFiServer server(80);
IPAddress local_IP(192, 168, 0, 164);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

WiFiClient client;

DHT dht(DHTPIN, DHTTYPE);

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
  dht.begin();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected with => ");
  Serial.print(WiFi.localIP());
  server.begin();
}


void loop(){
  request_from_wifi();
  if (int(ESP.getFreeHeap()) <= 2000){
    Serial.println("Restarting");
    ESP.restart();
  }
}

void request_from_wifi() {
  WiFiClient client = server.available();
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
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            if (header.indexOf("GET /temperature") >= 0) {
              StaticJsonBuffer<200> jsonBuffer;
              JsonObject& root = jsonBuffer.createObject();
              root["temperature_sensor"] = getTemperature();
              root.printTo(client);
            }
            else if (header.indexOf("GET /humidity") >= 0) {
              StaticJsonBuffer<200> jsonBuffer;
              JsonObject& root = jsonBuffer.createObject();
              root["humidity_sensor"] = getHumidity();
              root.printTo(client);
            }
            else if (header.indexOf("GET /1/on") >= 0) {
              output1State = "on";
              digitalWrite(output1, LOW);
            }
            else if (header.indexOf("GET /1/off") >= 0) {
              output1State = "off";
              digitalWrite(output1, HIGH);
            }
            else if (header.indexOf("GET /2/on") >= 0) {
              output2State = "on";
              digitalWrite(output2, LOW);
            }
            else if (header.indexOf("GET /2/off") >= 0) {
              output2State = "off";
              digitalWrite(output2, HIGH);
            }
            else if (header.indexOf("GET /3/on") >= 0) {
              output3State = "on";
              digitalWrite(output3, LOW);
            }
            else if (header.indexOf("GET /3/off") >= 0) {
              output3State = "off";
              digitalWrite(output3, HIGH);
            }
            else if (header.indexOf("GET /restart") >= 0) {
              ESP.restart();
            }
            else if (header.indexOf("GET /on") >= 0) {
              output1State = "on";
              output2State = "on";
              output3State = "on";
              digitalWrite(output1, LOW);
              digitalWrite(output2, LOW);
              digitalWrite(output3, LOW);
            }
            else if (header.indexOf("GET /off") >= 0) {
              output1State = "off";
              output2State = "off";
              output3State = "off";
              digitalWrite(output1, HIGH);
              digitalWrite(output2, HIGH);
              digitalWrite(output3, HIGH);
            }
            else if (header.indexOf("GET /getState") >= 0) {
              StaticJsonBuffer<200> jsonBuffer;
              JsonObject& root = jsonBuffer.createObject();
              root["Shed Light"] = output1State;
              root["Light"] = output2State;
              root["Fan"] = output3State;
              root["Heap"] = String(ESP.getFreeHeap());
              root.printTo(client);
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