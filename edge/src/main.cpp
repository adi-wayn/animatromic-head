/**
 * @file main.cpp
 * @brief Main entry point for the edge system.
 */
#include <Arduino.h>

#include "controllers/ProtocolDispatcher.h"
#include "core/SystemTasks.h"
#include "esp_task_wdt.h"

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }

    Serial.println("\n\n-----------------------------------");
    Serial.println("SYSTEM BOOTING... (115200 Baud)");
    Serial.println("-----------------------------------\n");

    // ── Task Watchdog Timer ──
    // Timeout raised to 6s (from 3s) to accommodate Light Sleep phases
    // where tasks may be dormant longer than the old budget allowed.
    // Uses legacy 2-arg API for Arduino-ESP32 framework compatibility.
    ESP_ERROR_CHECK(esp_task_wdt_init(6, true));  // 6s timeout, panic on trigger

    // ─────────────────────────────────────────────────────────────────
    //  CORE 0 — Wi-Fi stack + I2S Audio DMA (High Priority)
    //  These tasks must never be blocked by kinematics.
    // ─────────────────────────────────────────────────────────────────
    xTaskCreatePinnedToCore(networkTask, "Network", 4096, NULL, 20, NULL, 0);
    xTaskCreatePinnedToCore(audioDownlinkTask, "Audio_Downlink", 8192, NULL, 20, NULL, 0);

    // ─────────────────────────────────────────────────────────────────
    //  CORE 1 — Kinematics, JSON, Idle Behaviors, Power Management
    //  Priority ordering (highest first):
    //    10: jsonParserTask  — must process commands with low latency
    //     5: kinematicsTask  — 60 Hz ISR-driven, deterministic timing
    //     5: telemetryTask   — low-frequency background reporting
    //     3: idleBehaviorTask — random saccades/blinks, deferrable
    //     2: staggeredBootTask — runs once at boot, then self-deletes
    //     1: powerWatchdogTask — lowest priority, background monitoring
    // ─────────────────────────────────────────────────────────────────
    xTaskCreatePinnedToCore(jsonParserTask, "JSON_Parser", 4096, NULL, 10, NULL, 1);
    xTaskCreatePinnedToCore(kinematicsTask, "Kinematics", 4096, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(telemetryTask, "Telemetry", 4096, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(radarScannerTask, "Radar_Scan", 4096, NULL, 4, NULL, 1);
    xTaskCreatePinnedToCore(idleBehaviorTask, "Idle_Behavior", 4096, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(staggeredBootTask, "Boot", 4096, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(powerWatchdogTask, "Power_WDT", 2048, NULL, 1, NULL, 1);
}

void loop() {
    // Main loop is intentionally empty.
    // All logic is handled by dedicated FreeRTOS tasks.
    // With Tickless Idle enabled, this loop itself participates
    // in the FreeRTOS idle hook and contributes to Light Sleep.
    vTaskDelay(pdMS_TO_TICKS(1000));
}
