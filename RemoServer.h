#include <ESP8266WebServer.h> // HTTP
#include "WoLService.h"
#ifndef HEADER_WEBSERVER
  #define HEADER_WEBSERVER

  class RemoServer{
    public:
    void BeginServer();
    void HandleLoop();

    private:
    #if defined(SECURE_SERVER)
      // using Server = // fill when secure WebServer class completed
    #else
      using Server = ESP8266WebServer;
    #endif
    std::unique_ptr<Server> server;
    WoLService wol;

    // function prototypes for HTTP handlers
    void handleWakeUp(); 
    void handleNotFound();
    void handleSetMAC();

    unsigned long previousMillis = 0; // for multi-tasking Packet listening.
    unsigned long interval = 5000; // try every 5s.
  };

#endif
