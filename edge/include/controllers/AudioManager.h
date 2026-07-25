#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <Arduino.h>
#include <WiFiUdp.h>
#include <driver/i2s.h>
#include "hardware/I2SAudioConfig.h"
#include "controllers/ProtocolParser.h"

class AudioManager {
public:
    // --- Singleton Access ---
    static AudioManager& getInstance() {
        static AudioManager instance;
        return instance;
    }

    // Prevent copying
    AudioManager(const AudioManager&) = delete;
    void operator=(const AudioManager&) = delete;

    // Initialize I2S0 (mic) and bind uplink UDP socket
    void beginMic();

    // Read one chunk of PCM from the INMP441 via I2S DMA (blocking until ready)
    size_t readMicChunk(uint8_t* buffer, size_t bufferSize);

    // Send a raw PCM chunk to the Host on port 4211
    void sendToHost(const uint8_t* data, size_t len);

    // Set the Host IP (learned from the first control packet received)
    void setHostAddress(IPAddress ip);

    // Check if we have a valid Host IP to send to
    bool hasHostAddress() const;

    // Initialize I2S1 (speaker) and bind downlink UDP socket
    void beginSpeaker();

    // Receive one raw PCM chunk from Host via UDP (non-blocking)
    // Returns bytes received, 0 if no packet available
    int receiveFromHost(uint8_t* buffer, size_t maxLen);

    // Write a raw PCM chunk to the MAX98357A via I2S1 DMA (blocking)
    void writeToSpeaker(const uint8_t* data, size_t len);

    // Zero the I2S1 DMA buffer (for EMERGENCY_STOP silence)
    void flushSpeaker();

private:
    AudioManager();

    WiFiUDP uplinkSocket;       // Sends mic audio on port 4211
    WiFiUDP downlinkSocket;     // Receives TTS audio on port 4212
    IPAddress hostIP;
    bool hostIPKnown = false;
};

#endif
