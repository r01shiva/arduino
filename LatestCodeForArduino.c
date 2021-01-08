#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
// #include "Adafruit_MQTT.h"
// #include "Adafruit_MQTT_Client.h"
//#include "NewPing.h"
#include "DHT.h"
// #include "PIR.h"

#define WLAN_SSID       "Snap!"
#define WLAN_PASS       "@Abhi98999"

//#define AIO_SERVER      "io.adafruit.com"
//#define AIO_SERVERPORT  1883                   // use 8883 for SSL
//#define AIO_USERNAME    "r01shiva"
//#define AIO_KEY         "aio_hzXY903RSs1hnNP6J1gtfZrGHWnI"

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
// String output4State = "off";
String pirState = "off";

WiFiServer server(80);
IPAddress local_IP(192, 168, 0, 164);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

WiFiClient client;
//Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
//Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/Room Light");

DHT dht(DHTPIN, DHTTYPE);
// PIR pir(pir_sig);

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

// void setBulb(){
//   bool pirVal = pir.read();
//   if (pirVal == 1) {
//     digitalWrite(output2, LOW);
//     output2State = "on";
//   }
//   else if (pirVal == 0) {
//     digitalWrite(output2, HIGH);
//     output2State = "off";
//   }
// }

void setup() {
  pinMode(output1, OUTPUT);
  pinMode(output2, OUTPUT);
  pinMode(output3, OUTPUT);
  // pinMode(output4, OUTPUT);
  digitalWrite(output1, HIGH);
  digitalWrite(output2, HIGH);
  digitalWrite(output3, HIGH);
  // digitalWrite(output4, HIGH);
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
//  Serial.println("IP address: ");
  Serial.print(WiFi.localIP());
  server.begin();
  // mqtt.subscribe(&onoffbutton);
//  Serial.println("setup");
}

// uint32_t x=0;

void loop(){
  request_from_wifi();
  // request_from_mqtt();
  // if (pirState == "on"){
  //   setBulb();
  // }
//  delay(5000);
//  Serial.println(int(ESP.getFreeHeap()));
//  if (int(ESP.getFreeHeap()) <= 46000){
//    Serial.println("Restarting");
//    ESP.restart();
//  }
}

void request_from_wifi() {
  WiFiClient client = server.available();
  if (client) {                             // If a new client connects,
    // Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        // Serial.write(c);                    // print it out the serial monitor
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

            if (header.indexOf("GET /temperature") >= 0) {
              StaticJsonBuffer<200> jsonBuffer;
              JsonObject& root = jsonBuffer.createObject();
              // Serial.println("Getting temperature");
              // root["temperature"] = "temperature_sensor";
              // JsonArray& data = root.createNestedArray("data");
              // data.add(getTemperature());
              // root["value"] = getTemperature();
              root["temperature_sensor"] = getTemperature();
              // root.printTo(Serial);
              root.printTo(client);
            }
            else if (header.indexOf("GET /humidity") >= 0) {
              StaticJsonBuffer<200> jsonBuffer;
              JsonObject& root = jsonBuffer.createObject();
              // Serial.println("Getting humidity");
              // root["humdity"] = "humidity_sensor";
              // JsonArray& data = root.createNestedArray("data");
              // data.add(getHumidity());
              root["humidity_sensor"] = getHumidity();
              // root.printTo(Serial);
              root.printTo(client);
            }
            // else if (header.indexOf("GET /pir") >= 0) {
            //   StaticJsonBuffer<200> jsonBuffer;
            //   JsonObject& root = jsonBuffer.createObject();
            //   // Serial.println("Getting PIR");
            //   root["PIR"] = "pir_sensor";
            //   // JsonArray& data = root.createNestedArray("data");
            //   // data.add(getHumidity());
            //   root["value"] = getPIR();
            //   Serial.print(F("Val: "));
            //   Serial.println(getPIR());
            //   root.printTo(Serial);
            //   root.printTo(client);
            // }
            // turns the GPIOs on and off
            else if (header.indexOf("GET /1/on") >= 0) {
              // Serial.println("GPIO 1 on");
              output1State = "on";
              digitalWrite(output1, LOW);
            }
            else if (header.indexOf("GET /1/off") >= 0) {
              // Serial.println("GPIO 1 off");
              output1State = "off";
              digitalWrite(output1, HIGH);
            }
            else if (header.indexOf("GET /2/on") >= 0) {
              // Serial.println("GPIO 2 on");
              output2State = "on";
              digitalWrite(output2, LOW);
            }
            else if (header.indexOf("GET /2/off") >= 0) {
              // Serial.println("GPIO 2 off");
              output2State = "off";
              digitalWrite(output2, HIGH);
            }
            else if (header.indexOf("GET /3/on") >= 0) {
              // Serial.println("GPIO 3 on");
              output3State = "on";
              digitalWrite(output3, LOW);
            }
            else if (header.indexOf("GET /3/off") >= 0) {
              // Serial.println("GPIO 3 off");
              output3State = "off";
              digitalWrite(output3, HIGH);
            }
            // else if (header.indexOf("GET /4/on") >= 0) {
            //   // Serial.println("GPIO 4 on");
            //  output4State = "on";
            //  digitalWrite(output4, LOW);
            // }
            // else if (header.indexOf("GET /4/off") >= 0) {
            //   // Serial.println("GPIO 4 off");
            //   output4State = "off";
            //   digitalWrite(output4, HIGH);
            //   break;
            // }
            else if (header.indexOf("GET /restart") >= 0) {
              ESP.restart();
            }
            else if (header.indexOf("GET /on") >= 0) {
              // Serial.println("GPIO 4 off");
              output1State = "on";
              output2State = "on";
              output3State = "on";
              // output4State = "on";
              digitalWrite(output1, LOW);
              digitalWrite(output2, LOW);
              digitalWrite(output3, LOW);
              // digitalWrite(output4, LOW);
            }
            else if (header.indexOf("GET /off") >= 0) {
              // Serial.println("GPIO 4 off");
              output1State = "off";
              output2State = "off";
              output3State = "off";
              // output4State = "off";
              digitalWrite(output1, HIGH);
              digitalWrite(output2, HIGH);
              digitalWrite(output3, HIGH);
              // digitalWrite(output4, HIGH);
            }
            // else if (header.indexOf("GET /pir/on") >= 0) {
            //   pirState = "on";
            // }
            // else if (header.indexOf("GET /pir/off") >= 0) {
            //   pirState = "off";
            //   digitalWrite(output2, HIGH);
            // }
            else if (header.indexOf("GET /getState") >= 0) {
              StaticJsonBuffer<200> jsonBuffer;
              JsonObject& root = jsonBuffer.createObject();
              root["PIR"] = pirState;
              root["Shed Light"] = output1State;
              root["Light"] = output2State;
              root["Fan"] = output3State;
              root["Heap"] = String(ESP.getFreeHeap());
              // root["Plug"] = output4State;
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
    // Serial.println("Client disconnected.");
    // Serial.println("");
  }
}

//void request_from_mqtt(){
//  MQTT_connect();
//  Adafruit_MQTT_Subscribe *subscription;
//  while ((subscription = mqtt.readSubscription(1000))) {
//    if (subscription == &onoffbutton) {
//      Serial.print(F("Got: "));
//      Serial.println((char *)onoffbutton.lastread);
//      uint16_t state = atoi((char *)onoffbutton.lastread);
//      if (state == 0) {
//        digitalWrite(output1, LOW);
//        Serial.print("LOW");
//      }
//      else if(state == 1) {
//        digitalWrite(output1, HIGH);
//        Serial.print("HIGH");
//      }
//      Serial.print(state);
//    }
//  }
//}

//void MQTT_connect() {
//  int8_t ret;
//
//  // Stop if already connected.
//  if (mqtt.connected()) {
//    return;
//  }
//
//  Serial.print("Connecting to MQTT... ");
//
//  uint8_t retries = 3;
//  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
//       Serial.println(mqtt.connectErrorString(ret));
//       Serial.println("Retrying MQTT connection in 5 seconds...");
//       mqtt.disconnect();
//       delay(1000);  // wait 5 seconds
//       retries--;
//       if (retries == 0) {
//         // basically die and wait for WDT to reset me
//         while (1);
//       }
//  }
//  Serial.println("MQTT Connected!");
//}
