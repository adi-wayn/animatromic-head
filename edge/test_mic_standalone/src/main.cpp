#include <Arduino.h>
#include <driver/i2s.h>
#include <math.h>

// INMP441 Microphone Pins
constexpr i2s_port_t I2S_MIC_PORT    = I2S_NUM_0;
constexpr int        I2S_MIC_WS_PIN  = 25;
constexpr int        I2S_MIC_SCK_PIN = 26;
constexpr int        I2S_MIC_SD_PIN  = 33;

float hpf_x_prev = 0.0f;
float hpf_y_prev = 0.0f;
float lpf_y_prev = 0.0f;

void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(10); }
    
    Serial.println("\n\n=== INMP441 ASCII Monitor ===");
    Serial.println("Speak into the mic. The bar will expand as you get louder.\n");

    i2s_config_t i2s_mic_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = 32000,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 256,
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
}

void loop() {
    uint8_t buffer[2048]; 
    size_t bytesRead = 0;
    
    esp_err_t err = i2s_read(I2S_MIC_PORT, buffer, sizeof(buffer), &bytesRead, portMAX_DELAY);
    
    if (err == ESP_OK && bytesRead > 0) {
        int32_t* rawSamples32 = (int32_t*)buffer;
        int numFrames = bytesRead / 8; 
        
        float sumSq = 0.0f;
        
        for (int i = 0; i < numFrames; i++) {
            int32_t sampleL = rawSamples32[i * 2];
            int32_t sampleR = rawSamples32[i * 2 + 1];
            int32_t sample = (abs(sampleL) > abs(sampleR)) ? sampleL : sampleR;
            
            int32_t sample24 = sample >> 8;
            float x = (float)sample24 / 8388608.0f;
            
            float hp_y = 0.944f * (hpf_y_prev + x - hpf_x_prev);
            hpf_x_prev = x;
            hpf_y_prev = hp_y;
            
            float lp_y = lpf_y_prev + 0.611f * (hp_y - lpf_y_prev);
            lpf_y_prev = lp_y;
            
            float amplified = lp_y * 64.0f; 
            int32_t val32 = (int32_t)(amplified * 32767.0f);
            
            if (val32 > 32767) val32 = 32767;
            else if (val32 < -32768) val32 = -32768;
            
            sumSq += ((float)val32 * (float)val32);
        }
        
        float rms = sqrt(sumSq / numFrames);
        
        // --- ASCII Bar Chart Generation ---
        int barLength = (int)(rms / 150.0f); // Scale factor
        if (barLength > 50) barLength = 50;  // Max width of 50 chars
        
        Serial.print("Vol: ");
        if (rms < 1000) Serial.print(" ");
        if (rms < 100) Serial.print(" ");
        if (rms < 10) Serial.print(" ");
        Serial.print((int)rms);
        Serial.print(" | ");
        
        // If it crosses your silence threshold (500), mark it with a different character or just show a long bar
        for (int i = 0; i < 50; i++) {
            if (i < barLength) {
                if (rms > 500) Serial.print("#"); // Loud enough to trigger system
                else Serial.print("=");           // Just background noise
            } else {
                Serial.print(" ");
            }
        }
        
        if (rms > 500) {
            Serial.print("  <-- SPEECH DETECTED!");
        }
        Serial.println();
    }
}
