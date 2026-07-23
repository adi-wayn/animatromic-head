#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "controllers/ProtocolParser.h"

class NetworkManager {
public:
    // --- Singleton Access ---
    static NetworkManager& getInstance() {
        static NetworkManager instance;
        return instance;
    }

    // Prevent copying
    NetworkManager(const NetworkManager&) = delete;
    void operator=(const NetworkManager&) = delete;

    void begin(uint16_t port = PORT_CONTROL);
    
    // To be called continuously in the Network Core 0 task
    void update(); 
    
    // Allows decoupled consumers to pull data without direct coupling
    bool getNextMessage(String &messageOut);
    
private:
    NetworkManager();
    WiFiUDP udp;
    uint16_t listenPort;
    char incomingPacket[1024]; 
    QueueHandle_t messageQueue;
};

#endif
