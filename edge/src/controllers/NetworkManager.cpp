#include "controllers/NetworkManager.h"
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include <esp_wifi.h>
#include "controllers/AudioManager.h"

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
        
        // Disable WiFi power save to prevent UDP buffer overflow (Error 12 / ENOMEM)
        esp_wifi_set_ps(WIFI_PS_NONE);
        
        Serial.print("[Network] IP Address: ");
        Serial.println(WiFi.localIP());
        _isConnected = true;
    }

    // Advertise via mDNS so the Host can find us by hostname
    if (MDNS.begin("animatronic-head")) {
        Serial.println("[Network] mDNS responder started: animatronic-head.local");
    } else {
        Serial.println("[Network] mDNS FAILED to start.");
    }

    udp.begin(listenPort);
    Serial.printf("[Network] Listening on UDP port %d\n", listenPort);
}

void NetworkManager::update() {
    int packetSize = udp.parsePacket();
    if (packetSize) {
        // Dynamically latch onto the Host IP on EVERY valid control packet
        AudioManager::getInstance().setHostAddress(udp.remoteIP());
        
        int len = udp.read(incomingPacket, sizeof(incomingPacket) - 1);
        if (len > 0) {
            incomingPacket[len] = '\0';
            // Push to queue without blocking. Drop packet if queue is full.
            xQueueSend(messageQueue, &incomingPacket, (TickType_t)0);
        }
    }
}

bool NetworkManager::getNextMessage(String &messageOut, uint32_t timeoutMs) {
    if (messageQueue == nullptr) return false;
    
    char buffer[1024];
    // timeoutMs=0 → non-blocking poll; timeoutMs>0 → CPU-yielding block
    TickType_t ticks = (timeoutMs == 0) ? 0 : pdMS_TO_TICKS(timeoutMs);
    if (xQueueReceive(messageQueue, &buffer, ticks) == pdPASS) {
        messageOut = String(buffer);
        return true;
    }
    return false;
}
