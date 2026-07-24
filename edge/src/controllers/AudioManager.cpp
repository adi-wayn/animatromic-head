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
