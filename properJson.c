#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
const char* ssid     = "Snap!";
const char* password = "@Abhi98999";
WiFiServer server(80);
String header;
const int trigP = 5;  //D4 Or GPIO-2 of nodemcu
const int echoP = 4;  //D3 Or GPIO-0 of nodemcu
float output4State = 0;
long duration;
float distance;
unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;


// Set your Static IP address
IPAddress local_IP(192, 168, 0, 164);
// Set your Gateway IP address
IPAddress gateway(192, 168, 0, 1);
//103.47.16.74

IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional

void setup() {
  pinMode(trigP, OUTPUT);
  pinMode(echoP, INPUT);
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
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
              String webPage;
              StaticJsonBuffer<500> jsonBuffer;
              JsonObject& root = jsonBuffer.createObject();
              // Serial.println("Getting distance");
              root["ultrasonic"] = "1";
              digitalWrite(trigP, LOW);   // Makes trigPin low
              delayMicroseconds(2);       // 2 micro second delay
              digitalWrite(trigP, HIGH);  // tigPin high
              delayMicroseconds(10);      // trigPin high for 10 micro seconds
              digitalWrite(trigP, LOW);   // trigPin low
              duration = pulseIn(echoP, HIGH);   //Read echo pin, time in microseconds
              distance = duration*0.0343/2;        //Calculating actual/real distance
              root["cm"] = distance;
              JsonArray& data = root.createNestedArray("data");
              data.add(distance);
              data.add('add more values');
              root.printTo(Serial);
              root.printTo(client);
              root.printTo(webPage);  //Store JSON in String variable
              server.send(200, "text/html", webPage);
              delay(500);
            }

            // // Display the HTML web page
            // client.println("<!DOCTYPE html><html>");
            // client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            // client.println("<link rel=\"icon\" href=\"data:,\">");
            // // CSS to style the on/off buttons
            // // Feel free to change the background-color and font-size attributes to fit your preferences
            // client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            // client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            // client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            // client.println(".button2 {background-color: #77878A;}</style></head>");
            // client.println("<body><h1>Distance</h1>");
            // client.println("<p>Current distance ");
            // client.println(distance);
            // client.println("</p>");
            // client.println("</body></html>");

            // The HTTP response ends with another blank line
            // client.println();
            // Break out of the while loop
            // break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
