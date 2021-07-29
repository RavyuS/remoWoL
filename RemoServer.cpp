#include "RemoServer.h"

void RemoServer::BeginServer(){
    wol.initialize();
    wol.SearchTargetMAC();
    server.reset(new Server(80));
    server->on("/wakeup", std::bind(&RemoServer::handleWakeUp, this));
    server->on("/setMAC", std::bind(&RemoServer::handleSetMAC,this));
    server->onNotFound(std::bind(&RemoServer::handleNotFound,this));

    server->begin();
}

void RemoServer::HandleLoop(){
    server->handleClient();
    
    unsigned long currentMillis = millis();
    if(!wol.isTargetSet() && (currentMillis - previousMillis > interval)){
    previousMillis = currentMillis;
    wol.SearchTargetMAC();
  }
}

void RemoServer::handleWakeUp(){
  if(wol.isTargetSet()){
    wol.SendMagicPacket();
    server->send(200, "text/plain", "Wakeup request received successfully!");
  }
  else{
    server->send(200,"text/plain", "Wakeup request received, but no target MAC address set. Either manually set MAC address with /setMAC or let device attempt to find MAC address and try again later.");
  }
  
}

void RemoServer::handleSetMAC(){
  server->send(200, "text/plain", "WIP.");
}

void RemoServer::handleNotFound(){
  server->send(404, "text/plain", "404: Not found");
}
