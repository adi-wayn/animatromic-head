/**
 * @file PowerManager.cpp
 * @brief Implementation of PowerManager.cpp.
 */
#include "core/PowerManager.h"

#include "controllers/AnimatronicHead.h"
#include "core/Config.h"
#include "esp_pm.h"
#include "hardware/PCA9685_Driver.h"

// ============================================================
//  PowerManager::begin()
//  Called once in main.cpp setup(). Registers the ESP-IDF
//  power management config so the CPU can dynamically scale
//  between CPU_FREQ_LOW and CPU_FREQ_FULL.
//  Creates the mic wakeup semaphore used by the Audio ISR.
// ============================================================
void PowerManager::begin() {
    // Create the binary semaphore the Audio ISR will signal on sound detection
    micWakeupSem = xSemaphoreCreateBinary();
    if (micWakeupSem == nullptr) {
        Serial.println("[PowerManager] ERROR: Failed to create micWakeupSem!");
    }

    // Configure ESP-IDF dynamic frequency scaling
    // Light Sleep is triggered automatically by FreeRTOS Tickless Idle
    // (CONFIG_FREERTOS_USE_TICKLESS_IDLE=1 in platformio.ini)
    esp_pm_config_esp32_t pm_config = {
        .max_freq_mhz = CPU_FREQ_FULL, .min_freq_mhz = CPU_FREQ_LOW, .light_sleep_enable = true};

    esp_err_t err = esp_pm_configure(&pm_config);
    if (err == ESP_OK) {
        Serial.printf("[PowerManager] Dynamic CPU scaling enabled: %d–%d MHz. Light Sleep: ON.\n",
                      CPU_FREQ_LOW, CPU_FREQ_FULL);
    } else {
        Serial.printf(
            "[PowerManager] WARNING: esp_pm_configure failed: %s. "
            "Power management disabled.\n",
            esp_err_to_name(err));
    }
}

// ============================================================
//  PowerManager::enterLowPowerIdle()
//  Transitions to LOW_POWER_IDLE:
//   - Detaches ALL 9 servos (stops holding current, saves ~3–4A)
//   - Sets system state to LOW_POWER_IDLE
//   - CPU will drop to 80 MHz automatically via ESP-IDF PM driver
//   - FreeRTOS Tickless Idle enters Light Sleep when all tasks blocked
// ============================================================
void PowerManager::enterLowPowerIdle() {
    if (_isLowPower)
        return;
    _isLowPower = true;

    Serial.println("[PowerManager] >>> Entering LOW_POWER_IDLE <<<");
    Serial.println("[PowerManager]     Detaching all servos (eliminating holding current).");
    Serial.printf("[PowerManager]     CPU will scale down to %d MHz via PM driver.\n",
                  CPU_FREQ_LOW);

    // Detach all 9 servo channels — stops PWM output, eliminates holding current
    detachServo(NECK_Y.channel);
    detachServo(NECK_ONE.channel);
    detachServo(NECK_ROLL.channel);
    detachServo(JAW_UD.channel);
    detachServo(JAW_LR.channel);
    detachServo(EYES_X.channel);
    detachServo(EYES_Y.channel);
    detachServo(EYELID_LEFT.channel);
    detachServo(EYELID_RIGHT.channel);

    AnimatronicHead::getInstance().setState(SystemState::LOW_POWER_IDLE);
}

// ============================================================
//  PowerManager::enterFullPower()
//  Wakes up from LOW_POWER_IDLE:
//   - Resets activity timestamp so inactivity timer restarts
//   - Sets state back to IDLE_LISTENING
//   - CPU scales back up to 240 MHz automatically via PM driver
//   - Servos re-attach on the next kinematics write cycle
// ============================================================
void PowerManager::enterFullPower() {
    if (!_isLowPower) {
        // Just refresh the activity timestamp even if already awake
        AnimatronicHead::getInstance().updateActivityTimestamp();
        return;
    }
    _isLowPower = false;

    Serial.println("[PowerManager] <<< Returning to FULL_ACTIVE >>>");
    Serial.printf("[PowerManager]     CPU scaling back to %d MHz.\n", CPU_FREQ_FULL);

    AnimatronicHead::getInstance().setState(SystemState::IDLE_LISTENING);
    AnimatronicHead::getInstance().updateActivityTimestamp();
}
