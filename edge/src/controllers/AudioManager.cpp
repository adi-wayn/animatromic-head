#include "controllers/AudioManager.h"

AudioManager::AudioManager() {}

void AudioManager::beginMic() {
    // --- Configure I2S0 for INMP441 (RX mode) ---
    i2s_config_t i2s_mic_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = AUDIO_SAMPLE_RATE_HZ,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
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
        .data_out_num = I2S_PIN_NO_CHANGE,  // Not transmitting
        .data_in_num  = I2S_MIC_SD_PIN
    };

    esp_err_t err = i2s_driver_install(I2S_MIC_PORT, &i2s_mic_config, 0, NULL);
    if (err != ESP_OK) {
        Serial.printf("[AudioManager] I2S0 driver install FAILED: %s\n", esp_err_to_name(err));
        return;
    }

    err = i2s_set_pin(I2S_MIC_PORT, &mic_pin_config);
    if (err != ESP_OK) {
        Serial.printf("[AudioManager] I2S0 pin config FAILED: %s\n", esp_err_to_name(err));
        return;
    }

    Serial.println("[AudioManager] I2S0 Microphone initialized (INMP441).");
}

size_t AudioManager::readMicChunk(uint8_t* buffer, size_t bufferSize) {
    size_t bytesRead = 0;
    esp_err_t err = i2s_read(I2S_MIC_PORT, buffer, bufferSize, &bytesRead, portMAX_DELAY);
    if (err != ESP_OK) {
        Serial.printf("[AudioManager] I2S read error: %s\n", esp_err_to_name(err));
        return 0;
    }
    return bytesRead;
}

void AudioManager::sendToHost(const uint8_t* data, size_t len) {
    if (!hostIPKnown) return;  // Silently skip until we know who to send to

    uplinkSocket.beginPacket(hostIP, PORT_AUDIO_UPLINK);
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
