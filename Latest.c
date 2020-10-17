#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "NewPing.h"
#include "DHT.h"

const char* ssid     = "Snap!";
const char* password = "@Abhi98999";

#define DHTPIN 2 // what digital pin we're connected to
const int trigP = 5;  //D4 Or GPIO-2 of nodemcu
const int echoP = 4;  //D3 Or GPIO-0 of nodemcu
const int output1 = 14;
const int output2 = 12;
const int output3 = 13;
const int output4 = 15;
const long timeoutTime = 2000;
#define DHTTYPE DHT11 // DHT 11
int echoTime;             // echo time
String header;
int maxdist = 200;        // set maximum scan distance (cm)
long duration;
float distance;
unsigned long currentTime = millis();
unsigned long previousTime = 0;
String output1State = "off";
String output2State = "off";
String output3State = "off";
String output4State = "off";

WiFiServer server(80);
IPAddress local_IP(192, 168, 0, 164);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

NewPing sonar(trigP, echoP, maxdist);
DHT dht(DHTPIN, DHTTYPE);

float getDistance() {
  NewPing hcsr04(trigP,echoP);
  int distance = hcsr04.ping_cm();
  return distance;
}

float getTemperature() {
  dht.begin();
  float temperature = dht.readTemperature();
  while (temperature < 0 ){
    temperature = dht.readTemperature();
  }
  return temperature;
}

float getHumidity() {
  dht.begin();
  float humidity = dht.readHumidity();
  while (humidity < 0 ){
    humidity = dht.readHumidity();
  }
  return humidity;
}

float getActualDistance() {
  dht.begin();
  echoTime = sonar.ping();          // echo time (μs)
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  float vsound = 331.3+(0.606*temperature)+(0.0124*humidity);
  distance = (echoTime/2.0)*vsound/10000; // distance between sensor and target
  return distance;
}

void setup() {
  pinMode(output1, OUTPUT);
  pinMode(output2, OUTPUT);
  pinMode(output3, OUTPUT);
  pinMode(output4, OUTPUT);
  digitalWrite(output1, HIGH);
  digitalWrite(output2, HIGH);
  digitalWrite(output3, HIGH);
  digitalWrite(output4, HIGH);
  pinMode(trigP, OUTPUT);
  pinMode(echoP, INPUT);
  dht.begin();
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(50);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients
  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            if (header.indexOf("GET /distance") >= 0) {
              StaticJsonBuffer<200> jsonBuffer;
              JsonObject& root = jsonBuffer.createObject();
              Serial.println("Getting distance");
              root["distance"] = "ultrasonic";
              JsonArray& data = root.createNestedArray("data");
              data.add(getDistance());
              data.add(getDistance());
              root.printTo(Serial);
              root.printTo(client);
            }
            else if (header.indexOf("GET /temperature") >= 0) {
              StaticJsonBuffer<200> jsonBuffer;
              JsonObject& root = jsonBuffer.createObject();
              Serial.println("Getting temperature");
              root["temperature"] = "temperature_sensor";
              // JsonArray& data = root.createNestedArray("data");
              // data.add(getTemperature());
              root["value"] = getTemperature();
              root.printTo(Serial);
              root.printTo(client);
            }
            else if (header.indexOf("GET /humidity") >= 0) {
              StaticJsonBuffer<200> jsonBuffer;
              JsonObject& root = jsonBuffer.createObject();
              Serial.println("Getting humidity");
              root["humdity"] = "humidity_sensor";
              // JsonArray& data = root.createNestedArray("data");
              // data.add(getHumidity());
              root["value"] = getHumidity();
              root.printTo(Serial);
              root.printTo(client);
            }
            else if (header.indexOf("GET /getActualDistance") >= 0) {
              StaticJsonBuffer<200> jsonBuffer;
              JsonObject& root = jsonBuffer.createObject();
              Serial.println("Getting actual distance");
              root["distance"] = "ultrasonic_temperature_sensor";
              JsonArray& data = root.createNestedArray("data");
              data.add(getActualDistance());
              root.printTo(Serial);
              root.printTo(client);
            }
            // turns the GPIOs on and off
            else if (header.indexOf("GET /1/on") >= 0) {
              Serial.println("GPIO 1 on");
              output1State = "on";
              digitalWrite(output1, LOW);
            }
            else if (header.indexOf("GET /1/off") >= 0) {
              Serial.println("GPIO 1 off");
              output1State = "off";
              digitalWrite(output1, HIGH);
            }
            else if (header.indexOf("GET /2/on") >= 0) {
              Serial.println("GPIO 2 on");
              output2State = "on";
              digitalWrite(output2, LOW);
            }
            else if (header.indexOf("GET /2/off") >= 0) {
              Serial.println("GPIO 2 off");
              output2State = "off";
              digitalWrite(output2, HIGH);
            }
            else if (header.indexOf("GET /3/on") >= 0) {
              Serial.println("GPIO 3 on");
              output3State = "on";
              digitalWrite(output3, LOW);
            }
            else if (header.indexOf("GET /3/off") >= 0) {
              Serial.println("GPIO 3 off");
              output3State = "off";
              digitalWrite(output3, HIGH);
            }
            else if (header.indexOf("GET /4/on") >= 0) {
              Serial.println("GPIO 4 on");
              output4State = "on";
              digitalWrite(output4, LOW);
            }
            else if (header.indexOf("GET /4/off") >= 0) {
              Serial.println("GPIO 4 off");
              output4State = "off";
              digitalWrite(output4, HIGH);
            }
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
