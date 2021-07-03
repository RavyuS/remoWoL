#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "WoLService.h"


ESP8266WebServer server(80); // Create a webserver object that listens for HTTP requests on port 80
WoLService wol;

void handleWakeUp(); // function prototype for HTTP handlers
void handleNotFound();
void handleSetMAC();

long previousMillis = 0; // for multi-tasking Packet listening.
long interval = 5000; // try every 5s.

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

  server.on("/wakeup", handleWakeUp);
  server.on("/setMAC", handleSetMAC);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  wol.initialize();
  Serial.println("Starting packet receive");
  wol.SearchTargetMAC();
//  Serial.println(mac);


}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();

  unsigned long currentMillis = millis();
  if(!wol.isTargetSet() && (currentMillis - previousMillis > interval)){
    previousMillis = currentMillis;
    wol.SearchTargetMAC();
  }
}

void handleWakeUp(){
  if(wol.isTargetSet()){
    wol.SendMagicPacket();
    server.send(200, "text/plain", "Wakeup request received successfully!");
  }
  else{
    server.send(200,"text/plain", "Wakeup request received, but no target MAC address set. Either manually set MAC address with /setMAC or let device attempt to find MAC address and try again later.");
  }
  
}

void handleSetMAC(){
  server.send(200, "text/plain", "WIP.");
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found");
}
