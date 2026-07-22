#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

// Network-specific configuration is localized here, not in the mechanical Config.h
constexpr uint16_t DEFAULT_UDP_PORT = 4210;

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

    void begin(uint16_t port = DEFAULT_UDP_PORT);
    
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
