#pragma once

/**
 * @file NetworkManager.h
 * @brief Header for NetworkManager.h.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "controllers/ProtocolParser.h"

class NetworkManager {
   public:
    // --- Singleton Access ---
    static NetworkManager& getInstance() {
        static NetworkManager instance;
        return instance;
    }

    /**
     * @brief Prevent copying
     */
    NetworkManager(const NetworkManager&) = delete;
    void operator=(const NetworkManager&) = delete;

    void begin(uint16_t port = PORT_CONTROL);

    /**
     * @brief To be called continuously in the Network Core 0 task
     */
    void update();

    /**
     * @brief Allows decoupled consumers to pull data without direct coupling.
     * timeoutMs > 0: BLOCKS for up to timeoutMs, yielding CPU (enables tickless idle)
     * timeoutMs = 0: Non-blocking poll (legacy behavior)
     */
    bool getNextMessage(String& messageOut, uint32_t timeoutMs = 0);

    bool isConnected() const {
        return _isConnected;
    }

   private:
    NetworkManager();
    WiFiUDP udp;
    uint16_t listenPort;
    char incomingPacket[1024];
    QueueHandle_t messageQueue;
    bool _isConnected = false;
    uint32_t lastBroadcastTime = 0;
    bool _hostFound = false;
};
