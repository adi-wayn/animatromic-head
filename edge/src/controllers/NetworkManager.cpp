#include "controllers/NetworkManager.h"
#include <ESPmDNS.h>
#include <esp_wifi.h>
#include "esp_task_wdt.h"
#include "controllers/AudioManager.h"

// ── SoftAP Configuration ──
static const char* AP_SSID     = "Edgar_AP";
static const char* AP_PASSWORD = "edgarpassword123";

// Static IP configuration for the Access Point
static const IPAddress AP_LOCAL_IP(192, 168, 4, 1);
static const IPAddress AP_GATEWAY(192, 168, 4, 1);
static const IPAddress AP_SUBNET(255, 255, 255, 0);

NetworkManager::NetworkManager() : messageQueue(nullptr) {}

void NetworkManager::begin(uint16_t port) {
    listenPort = port;
    messageQueue = xQueueCreate(5, sizeof(char[1024]));

    Serial.println("[Network] Configuring SoftAP mode...");

    // Configure static IP before starting the AP
    WiFi.softAPConfig(AP_LOCAL_IP, AP_GATEWAY, AP_SUBNET);

    // Start the SoftAP with WPA2 authentication
    bool apStarted = WiFi.softAP(AP_SSID, AP_PASSWORD);

    // Wait for the AP interface to fully initialize.
    // Feed the watchdog on every iteration to prevent Core 0 TWDT panic.
    uint32_t apStartTime = millis();
    while (!apStarted && (millis() - apStartTime < 10000)) {
        esp_task_wdt_reset();
        Serial.println("[Network] Waiting for SoftAP to start...");
        vTaskDelay(pdMS_TO_TICKS(500));
        apStarted = WiFi.softAP(AP_SSID, AP_PASSWORD);
    }

    if (!apStarted) {
        Serial.println("[Network] FATAL: SoftAP failed to start after 10s. Restarting...");
        ESP.restart();
    }

    // Brief delay for DHCP server initialization; keep WDT happy
    esp_task_wdt_reset();
    vTaskDelay(pdMS_TO_TICKS(100));

    // Disable WiFi power save to prevent UDP buffer overflow (Error 12 / ENOMEM)
    esp_wifi_set_ps(WIFI_PS_NONE);

    _isConnected = true;

    // ── Print AP status to Serial ──
    Serial.println("\n[Network] ═══════════════════════════════════════");
    Serial.println("[Network]  SoftAP ACTIVE");
    Serial.printf("[Network]  SSID     : %s\n", AP_SSID);
    Serial.printf("[Network]  Password : %s\n", AP_PASSWORD);
    Serial.print("[Network]  AP IP    : ");
    Serial.println(WiFi.softAPIP());
    Serial.printf("[Network]  Subnet   : %s\n", AP_SUBNET.toString().c_str());
    Serial.println("[Network] ═══════════════════════════════════════\n");

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
    if (_isConnected && !_hostFound && millis() - lastBroadcastTime > 2000) {
        udp.beginPacket(IPAddress(192, 168, 4, 255), 4213);
        udp.print("ESP32_HEAD_HERE");
        udp.endPacket();
        lastBroadcastTime = millis();
    }

    int packetSize = udp.parsePacket();
    if (packetSize) {
        // Dynamically latch onto the Host IP on EVERY valid control packet
        AudioManager::getInstance().setHostAddress(udp.remoteIP());
        _hostFound = true; // Stop broadcasting once host talks to us!
        
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
