/**
 * @file test_mic_offline.cpp
 * @brief Implementation of test_mic_offline.cpp.
 */
#include <Arduino.h>
#include <WiFi.h>
#include <driver/i2s.h>

#define I2S_MIC_WS_PIN 25
#define I2S_MIC_SD_PIN 33
#define I2S_MIC_SCK_PIN 26
#define I2S_PORT I2S_NUM_0

const int SAMPLE_RATE = 16000;
const int RECORD_TIME_SEC = 2;
const int NUM_SAMPLES = SAMPLE_RATE * RECORD_TIME_SEC;
int16_t* audioBuffer;

void setup() {
    Serial.begin(115200);
    delay(1000);

    // 1. COMPLETELY DISABLE WI-FI (Kill all EMI/RF Interference)
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println("\n\n[Test] Wi-Fi is OFF. RF Interference eliminated.");

    // 2. Allocate RAM for 2 seconds of audio
    audioBuffer = (int16_t*)malloc(NUM_SAMPLES * sizeof(int16_t));
    if (!audioBuffer) {
        Serial.println("[Error] Could not allocate audio buffer!");
        while (1) delay(1000);
    }

    // 3. Configure I2S exactly as we did before
    i2s_config_t i2s_config = {.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
                               .sample_rate = SAMPLE_RATE,
                               .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
                               .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
                               .communication_format = I2S_COMM_FORMAT_STAND_I2S,
                               .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
                               .dma_buf_count = 8,
                               .dma_buf_len = 512,
                               .use_apll = false,
                               .tx_desc_auto_clear = false,
                               .fixed_mclk = 0};
    i2s_pin_config_t pin_config = {.bck_io_num = I2S_MIC_SCK_PIN,
                                   .ws_io_num = I2S_MIC_WS_PIN,
                                   .data_out_num = I2S_PIN_NO_CHANGE,
                                   .data_in_num = I2S_MIC_SD_PIN};

    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_PORT, &pin_config);

    Serial.println("[Test] I2S Initialized.");
    Serial.println("[Test] Get ready! Recording starts in 3 seconds...");
    delay(1000);
    Serial.println("[Test] 2...");
    delay(1000);
    Serial.println("[Test] 1...");
    delay(1000);
    Serial.println("[Test] RECORDING NOW! SPEAK!");

    // 4. Record 2 seconds into RAM
    size_t bytesRead = 0;
    int samplesRead = 0;
    int32_t raw32[256];  // Read buffer

    while (samplesRead < NUM_SAMPLES) {
        i2s_read(I2S_PORT, &raw32, sizeof(raw32), &bytesRead, portMAX_DELAY);
        int frames = bytesRead / 8;  // 8 bytes per frame (32-bit L + 32-bit R)

        for (int i = 0; i < frames; i++) {
            if (samplesRead >= NUM_SAMPLES)
                break;

            // Apply the 16x hardware gain (>> 6)
            int32_t L = raw32[i * 2] >> 6;

            // Clamp to 16-bit
            if (L > 32767)
                L = 32767;
            if (L < -32768)
                L = -32768;

            audioBuffer[samplesRead++] = (int16_t)L;
        }
    }

    Serial.println("[Test] RECORDING COMPLETE!");
    Serial.println("[Test] Dumping audio data to Serial Monitor (This takes a few seconds)...");
    Serial.println("---AUDIO_START---");

    // 5. Dump as comma-separated values
    for (int i = 0; i < NUM_SAMPLES; i++) {
        Serial.print(audioBuffer[i]);
        Serial.print(",");
        if (i % 20 == 19)
            Serial.println();  // 20 samples per line
    }

    Serial.println();
    Serial.println("---AUDIO_END---");
    Serial.println(
        "[Test] Dump complete. Please copy everything between AUDIO_START and AUDIO_END.");
}

void loop() {
    delay(1000);
}
