#include "controllers/NetworkManager.h"
#include <WiFiManager.h>

NetworkManager::NetworkManager() : messageQueue(nullptr) {}

void NetworkManager::begin(uint16_t port) {
    listenPort = port;
    messageQueue = xQueueCreate(5, sizeof(char[1024]));

    WiFiManager wm;
    Serial.println("[Network] Starting WiFiManager...");
    
    // Blocking autoConnect is fine here since Kinematics run on a separate core
    if(!wm.autoConnect("AnimatronicHead_AP")) {
        Serial.println("[Network] Failed to connect to Wi-Fi. Retrying...");
        ESP.restart();
    } else {
        Serial.println("\n[Network] Connected to Wi-Fi!");
        Serial.print("[Network] IP Address: ");
        Serial.println(WiFi.localIP());
    }

    udp.begin(listenPort);
    Serial.printf("[Network] Listening on UDP port %d\n", listenPort);
}

void NetworkManager::update() {
    int packetSize = udp.parsePacket();
    if (packetSize) {
        int len = udp.read(incomingPacket, sizeof(incomingPacket) - 1);
        if (len > 0) {
            incomingPacket[len] = '\0';
            // Push to queue without blocking. Drop packet if queue is full.
            xQueueSend(messageQueue, &incomingPacket, (TickType_t)0);
        }
    }
}

bool NetworkManager::getNextMessage(String &messageOut) {
    if (messageQueue == nullptr) return false;
    
    char buffer[1024];
    if (xQueueReceive(messageQueue, &buffer, 0) == pdPASS) {
        messageOut = String(buffer);
        return true;
    }
    return false;
}
