#include "controllers/AudioManager.h"

AudioManager::AudioManager() {}

void AudioManager::beginMic() {
    i2s_config_t i2s_mic_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = AUDIO_SAMPLE_RATE_HZ, // Now 32000Hz to fix INMP441 PLL
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT, // Stereo avoids ESP32 ONLY_LEFT zero-bug
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = I2S_DMA_BUF_COUNT,
        .dma_buf_len = I2S_DMA_BUF_LEN,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t mic_pin_config = {
        .bck_io_num   = I2S_MIC_SCK_PIN,
        .ws_io_num    = I2S_MIC_WS_PIN,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num  = I2S_MIC_SD_PIN
    };

    i2s_driver_install(I2S_MIC_PORT, &i2s_mic_config, 0, NULL);
    i2s_set_pin(I2S_MIC_PORT, &mic_pin_config);
    Serial.println("[AudioManager] I2S0 Microphone initialized (INMP441 - 32-bit 32kHz Mode).");
}

size_t AudioManager::readMicChunk(uint8_t* buffer, size_t bufferSize) {
    size_t bytesRead = 0;
    esp_err_t err = i2s_read(I2S_MIC_PORT, buffer, bufferSize, &bytesRead, portMAX_DELAY);
    if (err != ESP_OK || bytesRead == 0) return 0;
    
    int32_t* rawSamples32 = (int32_t*)buffer;
    int16_t* cleanSamples16 = (int16_t*)buffer;
    int numFrames = bytesRead / 8; // 32-bit RIGHT_LEFT = 8 bytes per frame
    
    const float hp_alpha = 0.944f; 
    const float lp_alpha = 0.611f; 
    
    for (int i = 0; i < numFrames; i++) {
        // Read both channels. One has valid 24-bit audio (huge numbers), 
        // the other is floating (tiny thermal noise numbers).
        int32_t sampleL = rawSamples32[i * 2];
        int32_t sampleR = rawSamples32[i * 2 + 1];
        
        // Auto-select the active channel (bypasses DMA swap bugs and L/R wiring issues)
        int32_t sample = (abs(sampleL) > abs(sampleR)) ? sampleL : sampleR;
        
        // INMP441 outputs 24-bit audio in a 32-bit slot, MSB-aligned.
        // Shift right by 8 to extract the exact 24-bit value, preserving the sign bit.
        int32_t sample24 = sample >> 8;
        
        // Normalize 24-bit signed integer to float [-1.0, 1.0] by dividing by 2^23 (8388608.0f)
        float x = (float)sample24 / 8388608.0f;
        
        // 1. High-Pass Filter (150Hz DC Blocker)
        float hp_y = hp_alpha * (hpf_y_prev + x - hpf_x_prev);
        hpf_x_prev = x;
        hpf_y_prev = hp_y;
        
        // 2. Low-Pass Filter (4000Hz Static Remover)
        float lp_y = lpf_y_prev + lp_alpha * (hp_y - lpf_y_prev);
        lpf_y_prev = lp_y;
        
        // 3. Digital Gain (64.0x) to boost quiet INMP441 speech
        float amplified = lp_y * 64.0f; 
        
        // 4. Quantize to 16-bit PCM and clamp to prevent overflow
        int32_t val32 = (int32_t)(amplified * 32767.0f);
        if (val32 > 32767) val32 = 32767;
        else if (val32 < -32768) val32 = -32768;
        
        cleanSamples16[i] = (int16_t)val32;
    }
    
    // We compressed 64-bit frames (8 bytes) into 16-bit mono frames (2 bytes), return exactly that!
    return numFrames * 2;
}

void AudioManager::sendToHost(const uint8_t* data, size_t len) {
    if (!hostIPKnown) return;  // Silently skip until we know who to send to

    uint32_t timestamp = millis();
    
    // Header format: 2 bytes seq_num, 4 bytes timestamp
    uint8_t header[6];
    header[0] = (uplinkSeqNum >> 8) & 0xFF;
    header[1] = uplinkSeqNum & 0xFF;
    header[2] = (timestamp >> 24) & 0xFF;
    header[3] = (timestamp >> 16) & 0xFF;
    header[4] = (timestamp >> 8) & 0xFF;
    header[5] = timestamp & 0xFF;
    
    uplinkSeqNum++;

    uplinkSocket.beginPacket(hostIP, PORT_AUDIO_UPLINK);
    uplinkSocket.write(header, 6);
    uplinkSocket.write(data, len);
    uplinkSocket.endPacket();
}

void AudioManager::setHostAddress(IPAddress ip) {
    hostIP = ip;
    hostIPKnown = true;
    Serial.printf("[AudioManager] Host address set to: %s\n", ip.toString().c_str());
}

bool AudioManager::hasHostAddress() const {
    return hostIPKnown;
}

IPAddress AudioManager::getHostAddress() const {
    return hostIP;
}

void AudioManager::beginSpeaker() {
    // --- Configure I2S1 for MAX98357A (TX mode) ---
    i2s_config_t i2s_spk_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = AUDIO_SAMPLE_RATE_HZ,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = I2S_DMA_BUF_COUNT,
        .dma_buf_len = I2S_DMA_BUF_LEN,
        .use_apll = false,
        .tx_desc_auto_clear = true,   // Output silence on buffer underflow
        .fixed_mclk = 0
    };

    i2s_pin_config_t spk_pin_config = {
        .bck_io_num   = I2S_SPK_BCK_PIN,
        .ws_io_num    = I2S_SPK_LRC_PIN,
        .data_out_num = I2S_SPK_DIN_PIN,
        .data_in_num  = I2S_PIN_NO_CHANGE   // Not receiving
    };

    esp_err_t err = i2s_driver_install(I2S_SPK_PORT, &i2s_spk_config, 0, NULL);
    if (err != ESP_OK) {
        Serial.printf("[AudioManager] I2S1 driver install FAILED: %s\n", esp_err_to_name(err));
        return;
    }

    err = i2s_set_pin(I2S_SPK_PORT, &spk_pin_config);
    if (err != ESP_OK) {
        Serial.printf("[AudioManager] I2S1 pin config FAILED: %s\n", esp_err_to_name(err));
        return;
    }

    // Bind downlink UDP socket
    downlinkSocket.begin(PORT_AUDIO_DOWNLINK);

    Serial.println("[AudioManager] I2S1 Speaker initialized (MAX98357A).");
    Serial.printf("[AudioManager] Listening for TTS audio on UDP port %d\n", PORT_AUDIO_DOWNLINK);
}

int AudioManager::receiveFromHost(uint8_t* buffer, size_t maxLen) {
    int packetSize = downlinkSocket.parsePacket();
    if (packetSize > 0) {
        return downlinkSocket.read(buffer, maxLen);
    }
    return 0;
}

void AudioManager::writeToSpeaker(const uint8_t* data, size_t len) {
    size_t bytesWritten = 0;
    i2s_write(I2S_SPK_PORT, data, len, &bytesWritten, portMAX_DELAY);
}

void AudioManager::flushSpeaker() {
    i2s_zero_dma_buffer(I2S_SPK_PORT);
    Serial.println("[AudioManager] Speaker DMA flushed (silence).");
}

void AudioManager::setAmplitude(float intensity) {
    // Clamp between 0.0 and 1.0
    if (intensity < 0.0f) intensity = 0.0f;
    if (intensity > 1.0f) intensity = 1.0f;
    currentAmplitude = intensity;
}

float AudioManager::getAmplitude() const {
    return currentAmplitude;
}
