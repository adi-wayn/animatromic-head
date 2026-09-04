#pragma once

/**
 * @file AudioManager.h
 * @brief Header for AudioManager.h.
 */

#include <Arduino.h>
#include <WiFiUdp.h>
#include <driver/i2s.h>

#include "controllers/ProtocolParser.h"
#include "hardware/I2SAudioConfig.h"

class AudioManager {
   public:
    // --- Singleton Access ---
    static AudioManager& getInstance() {
        static AudioManager instance;
        return instance;
    }

    /**
     * @brief Prevent copying
     */
    AudioManager(const AudioManager&) = delete;
    void operator=(const AudioManager&) = delete;

    /**
     * @brief Initialize I2S0 (mic) and bind uplink UDP socket
     */
    void beginMic();

    /**
     * @brief Read one chunk of PCM from the INMP441 via I2S DMA (blocking until ready)
     */
    size_t readMicChunk(uint8_t* buffer, size_t bufferSize);

    /**
     * @brief Send a raw PCM chunk to the Host on port 4211
     */
    void sendToHost(const uint8_t* data, size_t len);

    /**
     * @brief Set the Host IP (learned from the first control packet received)
     */
    void setHostAddress(IPAddress ip);

    /**
     * @brief Check if we have a valid Host IP to send to
     */
    bool hasHostAddress() const;

    /**
     * @brief Get the Host IP Address
     */
    IPAddress getHostAddress() const;

    /**
     * @brief Initialize I2S1 (speaker) and bind downlink UDP socket
     */
    void beginSpeaker();

    /**
     * @brief Receive one raw PCM chunk from Host via UDP (non-blocking)
     * Returns bytes received, 0 if no packet available
     */
    int receiveFromHost(uint8_t* buffer, size_t maxLen);

    /**
     * @brief Write a raw PCM chunk to the MAX98357A via I2S1 DMA (blocking)
     */
    void writeToSpeaker(const uint8_t* data, size_t len);

    /**
     * @brief Play a local PCM clip (blocking or chunked)
     */
    void playLocalClip(const uint8_t* pcmData, size_t len);

    /**
     * @brief Cancel any currently playing local clip
     */
    void cancelLocalPlayback();

    /**
     * @brief Zero the I2S1 DMA buffer (for EMERGENCY_STOP silence)
     */
    void flushSpeaker();

    /**
     * @brief Set the current audio playback amplitude (normalized 0.0 to 1.0)
     */
    void setAmplitude(float intensity);

    /**
     * @brief Get the current audio playback amplitude
     */
    float getAmplitude() const;

   private:
    AudioManager();

    WiFiUDP uplinkSocket;    // Sends mic audio on port 4211
    WiFiUDP downlinkSocket;  // Receives TTS audio on port 4212
    IPAddress hostIP;
    bool hostIPKnown = false;
    uint16_t uplinkSeqNum = 0;

    /**
     * @brief High-Pass Filter state for DC offset removal
     */
    float hpf_x_prev = 0.0f;
    float hpf_y_prev = 0.0f;

    /**
     * @brief Low-Pass Filter state
     */
    float lpf_y_prev = 0.0f;

    /**
     * @brief 32-bit floats are naturally atomic on ESP32, but we use volatile
     * to prevent compiler caching across cores.
     */
    volatile float currentAmplitude = 0.0f;
    volatile bool isLocalPlaybackCancelled = false;
};
