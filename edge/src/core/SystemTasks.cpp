#include "core/SystemTasks.h"
#include <Arduino.h>
#include "controllers/AnimatronicHead.h"
#include "controllers/NetworkManager.h"
#include "controllers/AudioManager.h"
#include "hardware/PCA9685_Driver.h"
#include "core/PowerManager.h"
#include <math.h>
#include "esp_task_wdt.h"

// ============================================================
//  Hardware Timer ISR — 60 Hz Kinematics Clock
//  IRAM_ATTR: runs from IRAM so it is safe during cache misses
//  and during Light Sleep wakeup transitions.
// ============================================================
SemaphoreHandle_t kinematicsTriggerSem = nullptr;
static hw_timer_t* kinematicsTimer = nullptr;

void IRAM_ATTR onKinematicsTimer() {
    // ISR context: give the binary semaphore to unblock kinematicsTask
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(kinematicsTriggerSem, &xHigherPriorityTaskWoken);
    // If giving the semaphore unblocked a higher-priority task, yield immediately
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// ============================================================
//  Audio DMA ISR — Microphone Wakeup Interrupt
//
//  This is the KEY external hardware interrupt you asked for.
//  When the system is in LOW_POWER_IDLE, the I2S DMA continues
//  to fill buffers. The I2S driver's internal ISR fires on each
//  DMA completion. We hook into this via a FreeRTOS task that
//  checks RMS amplitude. When RMS exceeds SILENCE_RMS_THRESHOLD,
//  it gives the PowerManager::micWakeupSem semaphore, which
//  the audioUplinkTask is blocked on.
//
//  Because the INMP441 is an I2S device (not a GPIO pin), there
//  is no dedicated "audio detected" GPIO we can attach a
//  hardware EXTI interrupt to. The correct approach is:
//    1. I2S DMA ISR → triggers i2s_read to return data
//    2. Task reads buffer, computes RMS
//    3. If RMS > threshold → give micWakeupSem → wakeup
//
//  This is functionally equivalent to an external interrupt:
//  the task is blocked (CPU in tickless idle / Light Sleep)
//  until real audio energy arrives.
// ============================================================

// ============================================================
//  Task 1: Kinematics (Core 1, Priority 5)
//  Triggered by 60 Hz hardware timer ISR instead of vTaskDelay.
//  CPU is in FreeRTOS tickless idle (Light Sleep eligible) while
//  blocked on xSemaphoreTake between timer firings.
// ============================================================
void kinematicsTask(void *pvParameters) {
    (void) pvParameters;

    // Create binary semaphore that ISR will signal
    kinematicsTriggerSem = xSemaphoreCreateBinary();
    if (kinematicsTriggerSem == nullptr) {
        Serial.println("[Kinematics] FATAL: Failed to create timer semaphore!");
        vTaskDelete(NULL);
        return;
    }

    // Setup hardware timer 0 at 80 prescaler → 1 tick = 1µs
    // 60 Hz → period = 16666 µs
    kinematicsTimer = timerBegin(0, 80, true);
    timerAttachInterrupt(kinematicsTimer, &onKinematicsTimer, true);
    timerAlarmWrite(kinematicsTimer, 16666, true); // auto-reload
    timerAlarmEnable(kinematicsTimer);

    Serial.println("[Kinematics] 60 Hz hardware timer ISR armed.");

    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));

    while (true) {
        ESP_ERROR_CHECK(esp_task_wdt_reset());

        // ── BLOCKING WAIT on timer semaphore ──
        // CPU goes into FreeRTOS tickless idle here.
        // The timer ISR (IRAM_ATTR) fires every 16.6ms and wakes us.
        // 100ms timeout acts as a TWDT-safe fallback if ISR misfires.
        xSemaphoreTake(kinematicsTriggerSem, pdMS_TO_TICKS(100));

        // Skip kinematic update if system is in low-power idle
        if (AnimatronicHead::getInstance().isInLowPowerIdle()) continue;

        if (AnimatronicHead::getInstance().isBooted()) {
            // Drive lip sync from audio amplitude during speech
            if (AnimatronicHead::getInstance().getState() == SystemState::SPEAKING_SYNCING) {
                float intensity = AudioManager::getInstance().getAmplitude();
                PoseController::getInstance().syncJawToAmplitude(intensity);
            }
            AnimatronicHead::getInstance().updateKinematics();
        }
    }
}

// ============================================================
//  Boot Task (Core 1, Priority 2) — runs once then deletes itself
// ============================================================
void staggeredBootTask(void *pvParameters) {
    (void) pvParameters;
    Serial.println("[Boot] Starting Staggered Boot Sequence...");

    AnimatronicHead::getInstance().begin();

    ServoConfig servos[] = {NECK_ONE, NECK_Y, NECK_ROLL, EYES_X, EYES_Y, JAW_UD, JAW_LR, EYELID_LEFT, EYELID_RIGHT};
    int numServos = sizeof(servos) / sizeof(ServoConfig);

    for (int i = 0; i < numServos; i++) {
        Serial.printf("[Boot] Initializing Servo Channel %d...\n", servos[i].channel);
        double initAngle = servos[i].centerAngle;
        if (servos[i].channel == EYELID_LEFT.channel)  initAngle = EYELID_LEFT.minAngle;
        if (servos[i].channel == EYELID_RIGHT.channel) initAngle = EYELID_RIGHT.minAngle;
        safeSetServoAngle(servos[i].channel, initAngle, servos[i].minAngle, servos[i].maxAngle);
        vTaskDelay(pdMS_TO_TICKS(500)); // 500ms stagger per servo
    }

    Serial.println("[Boot] Staggered Boot Complete. System Ready.");
    AnimatronicHead::getInstance().setBooted(true);
    AnimatronicHead::getInstance().updateActivityTimestamp(); // Mark boot as activity
    vTaskDelete(NULL);
}

// ============================================================
//  Task 2: Network Transport (Core 0, Priority 20)
// ============================================================
void networkTask(void *pvParameters) {
    (void) pvParameters;
    NetworkManager& network = NetworkManager::getInstance();
    network.begin();
    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
    while (true) {
        ESP_ERROR_CHECK(esp_task_wdt_reset());
        network.update();
        // Brief yield: i2s interrupt-driven anyway on Core 0
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}


// ============================================================
//  Task 4: Idle Behaviors (Core 1, Priority 3)
//  Suspended entirely during LOW_POWER_IDLE.
// ============================================================
void idleBehaviorTask(void *pvParameters) {
    (void) pvParameters;
    while (true) {
        AnimatronicHead& head = AnimatronicHead::getInstance();

        // In low-power mode: sleep long, do nothing
        if (head.isInLowPowerIdle()) {
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }

        if (head.isBooted() && head.getState() == SystemState::IDLE_LISTENING) {
            head.triggerSaccade(millis());

            // 15% chance to blink
            if (random(0, 100) < 15) {
                head.executePose("BLINK");
                vTaskDelay(pdMS_TO_TICKS(150)); // Wait for eyelids to close
                head.executePose("UNBLINK");
            }

            // Biological delay between idle movements
            vTaskDelay(pdMS_TO_TICKS(random(800, 2500)));
        } else {
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
}

// ============================================================
//  Task 5: Audio Uplink — Mic to Host (Core 0, Priority 20)
//
//  HARDWARE INTERRUPT WAKEUP MECHANISM:
//  When in LOW_POWER_IDLE:
//    - i2s_read still blocks on DMA (natural yield, DMA ISR driven)
//    - We compute RMS of each incoming buffer
//    - If RMS > SILENCE_RMS_THRESHOLD: give PowerManager::micWakeupSem
//      → this acts as the HARDWARE INTERRUPT WAKEUP from the mic
//    - audioUplinkTask itself blocks on micWakeupSem when silent
//      → CPU enters tickless idle / Light Sleep between DMA completions
//
//  When in IDLE_LISTENING / FULL_ACTIVE:
//    - Stream all audio to host (normal operation)
//    - If RMS is consistently below threshold: skip network write
//      (silence suppression — saves bandwidth when room is quiet)
// ============================================================
void audioUplinkTask(void *pvParameters) {
    (void) pvParameters;

    // Block until network is fully initialized to prevent LwIP socket crashes
    while (!NetworkManager::getInstance().isConnected()) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    AudioManager& audio = AudioManager::getInstance();
    audio.beginMic();

    uint8_t pcmBuffer[AUDIO_CHUNK_SIZE_BYTES];

    // Setup built-in LED for visual audio detection feedback
    pinMode(2, OUTPUT);
    int debugPrintCounter = 0;

    bool isSendingAudio = false;
    uint32_t lastSendTime = 0;
    uint32_t lastActiveLogTime = 0;

    while (true) {
        // i2s_read blocks on DMA interrupt — natural CPU yield
        // even during Light Sleep, the I2S DMA continues running
        size_t bytesRead = audio.readMicChunk(pcmBuffer, AUDIO_CHUNK_SIZE_BYTES);

        if (bytesRead > 0) {
            // readMicChunk now returns purely 16-bit PCM data.
            int16_t* cleanSamples16 = (int16_t*)pcmBuffer;
            int numFrames = bytesRead / 2;
            
            float sumSq = 0.0f;
            for (int i = 0; i < numFrames; i++) {
                float s = (float)cleanSamples16[i];
                sumSq += s * s;
            }
            float rms = (numFrames > 0) ? sqrtf(sumSq / numFrames) : 0.0f;

            // 2. Threshold Check
            // We only flag 'soundDetected' if the audio energy is above the noise floor
            // 216 RMS is the thermal noise floor for this defective module.
            // A threshold of 500+ is needed for real speech.
            bool soundDetected = (rms > SILENCE_RMS_THRESHOLD);
            digitalWrite(2, soundDetected ? HIGH : LOW);

            if (soundDetected) {
                if (AnimatronicHead::getInstance().isInLowPowerIdle()) {
                    SemaphoreHandle_t sem = PowerManager::getInstance().micWakeupSem;
                    if (sem != nullptr) {
                        xSemaphoreGive(sem);
                        Serial.println("[AudioUplink] Audio interrupt: waking from LOW_POWER_IDLE.");
                    }
                }
                AnimatronicHead::getInstance().updateActivityTimestamp();
            }

            // Stream continuous 16-bit Mono audio to the host
            if (!AnimatronicHead::getInstance().isInLowPowerIdle()) {
                if (soundDetected) {
                    if (!isSendingAudio) {
                        Serial.println("[AudioUplink] START: Capturing audio from INMP441 and sending to Host...");
                        isSendingAudio = true;
                    }
                    audio.sendToHost((uint8_t*)cleanSamples16, numFrames * 2);
                    lastSendTime = millis();

                    if (millis() - lastActiveLogTime > 2000) {
                        Serial.println("[AudioUplink] ACTIVE: Currently streaming mic data over network.");
                        lastActiveLogTime = millis();
                    }
                } else if (isSendingAudio && (millis() - lastSendTime > 1000)) {
                    Serial.println("[AudioUplink] END: Silence detected. Stopped sending mic data.");
                    isSendingAudio = false;
                }
            }
        }
    }
}

// ============================================================
//  Task 6: Audio Downlink — Host TTS to Speaker (Core 0, Priority 20)
//  Amplitude decay for lip sync preserved. Activity timestamp
//  updated on every received audio packet.
// ============================================================
void audioDownlinkTask(void *pvParameters) {
    (void) pvParameters;

    // Block until network is fully initialized to prevent LwIP socket crashes
    while (!NetworkManager::getInstance().isConnected()) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    AudioManager& audio = AudioManager::getInstance();
    audio.beginSpeaker();

    uint8_t pcmBuffer[AUDIO_CHUNK_SIZE_BYTES];
    const float RMS_SCALER = 32768.0f;

    bool isReceivingAudio = false;
    uint32_t lastReceiveTime = 0;
    uint32_t lastActiveLogTime = 0;

    while (true) {
        // Skip playback in low-power mode (no TTS expected)
        if (AnimatronicHead::getInstance().isInLowPowerIdle()) {
            vTaskDelay(pdMS_TO_TICKS(200));
            continue;
        }

        int bytesReceived = audio.receiveFromHost(pcmBuffer, AUDIO_CHUNK_SIZE_BYTES);
        if (bytesReceived > 0) {
            if (!isReceivingAudio) {
                Serial.println("[AudioDownlink] START: Receiving TTS audio from Host. Sending to MAX98357A...");
                isReceivingAudio = true;
            }
            lastReceiveTime = millis();

            if (millis() - lastActiveLogTime > 2000) {
                Serial.println("[AudioDownlink] ACTIVE: Currently playing TTS audio through MAX98357A.");
                lastActiveLogTime = millis();
            }

            // Wake from low power if a downlink packet arrives unexpectedly
            PowerManager::getInstance().enterFullPower();
            AnimatronicHead::getInstance().updateActivityTimestamp();

            // Calculate RMS for Lip-Sync
            int16_t* samples = (int16_t*)pcmBuffer;
            int numSamples   = bytesReceived / 2;
            float sumSquares = 0.0f;
            for (int i = 0; i < numSamples; i++) {
                float sample = (float)samples[i];
                sumSquares += sample * sample;
            }
            float rms = (numSamples > 0) ? sqrtf(sumSquares / numSamples) : 0.0f;
            float intensity = (rms / RMS_SCALER) * 3.0f;
            audio.setAmplitude(intensity);

            audio.writeToSpeaker(pcmBuffer, bytesReceived);
        } else {
            if (isReceivingAudio && (millis() - lastReceiveTime > 500)) {
                Serial.println("[AudioDownlink] END: Audio stream finished. Stopped sending to MAX98357A.");
                isReceivingAudio = false;
            }

            // Amplitude decay: prevents jaw snapping on dropped packets
            // We loop every ~1ms. Packets arrive every ~32ms.
            // 0.95^32 = 0.19 (gradual decay instead of instant 0.8^32 = 0.0007)
            float currentAmp = audio.getAmplitude();
            if (currentAmp > 0.01f) {
                audio.setAmplitude(currentAmp * 0.95f);
            } else {
                audio.setAmplitude(0.0f);
            }
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
}

// ============================================================
//  Task 7: Telemetry (Core 1, Priority 5)
//  Suspended in low-power idle to save CPU and network.
// ============================================================
void telemetryTask(void *pvParameters) {
    (void) pvParameters;
    WiFiUDP telemetrySocket;

    while (true) {
        // Skip telemetry in low-power mode
        if (AnimatronicHead::getInstance().isInLowPowerIdle()) {
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }

        if (AudioManager::getInstance().hasHostAddress()) {
            IPAddress hostIP = AudioManager::getInstance().getHostAddress();

            String telemetry = "{\"type\":\"TELEMETRY\",\"cpu_load\":";
            telemetry += String(xPortGetFreeHeapSize());
            telemetry += ",\"power_state\":\"";
            telemetry += PowerManager::getInstance().isLowPower() ? "LOW_POWER" : "ACTIVE";
            telemetry += "\"}";

            telemetrySocket.beginPacket(hostIP, 4213);
            telemetrySocket.print(telemetry);
            telemetrySocket.endPacket();
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // 10 Hz
    }
}

// ============================================================
//  Task 8: Power Watchdog (Core 1, Priority 1 — lowest)
//  NEW TASK. Monitors inactivity timer. If no activity for
//  INACTIVITY_TIMEOUT_MS (60s), transitions to LOW_POWER_IDLE.
//  Also handles wakeup from LOW_POWER_IDLE when micWakeupSem
//  is signaled by the Audio ISR path above.
// ============================================================
void powerWatchdogTask(void *pvParameters) {
    (void) pvParameters;

    PowerManager& pm = PowerManager::getInstance();
    pm.begin(); // Initialize ESP-IDF PM driver + create micWakeupSem

    while (true) {
        if (pm.isLowPower()) {
            // ── BLOCKING WAIT for mic wakeup signal ──
            // CPU enters FreeRTOS tickless idle here → ESP32 enters Light Sleep.
            // The audioUplinkTask (running on Core 0 via DMA ISR) will give
            // micWakeupSem when audio energy is detected above threshold.
            // portMAX_DELAY = wait forever, woken only by the audio interrupt.
            if (xSemaphoreTake(pm.micWakeupSem, pdMS_TO_TICKS(10000)) == pdTRUE) {
                // Audio interrupt fired! Wake up the system.
                pm.enterFullPower();
                Serial.println("[PowerWDT] Mic audio interrupt: system waking up.");
            }
            // Loop back and recheck (handles 10s timeout fallback)
            continue;
        }

        // ── ACTIVE MODE: check inactivity timer ──
        if (AnimatronicHead::getInstance().isBooted()) {
            uint32_t now        = millis();
            uint32_t lastActMs  = AnimatronicHead::getInstance().getLastActivityMs();
            uint32_t idleTime   = now - lastActMs;

            if (idleTime >= INACTIVITY_TIMEOUT_MS) {
                Serial.printf("[PowerWDT] No activity for %lu ms. Entering LOW_POWER_IDLE.\n", idleTime);
                pm.enterLowPowerIdle();
            }
        }

        // Check every 5 seconds — very low CPU cost
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
