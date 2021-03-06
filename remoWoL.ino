#include <ESP8266WiFi.h>
#include "RemoServer.h"

RemoServer server; // Create a webserver wrapper object that listens for HTTP requests on port 80



void setup() {
  Serial.begin(115200);         // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');
  
  WiFi.begin();             // Connect to the network
  Serial.print("Connecting to network");
  
  Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }

  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer

  server.BeginServer();
  Serial.println("HTTP server started");


  Serial.println("Starting packet receive");



}

void loop() {
  // put your main code here, to run repeatedly:
  server.HandleLoop();


}
