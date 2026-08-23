#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// ============================================================
//  Inactivity timeout before entering LOW_POWER_IDLE
// ============================================================
constexpr uint32_t INACTIVITY_TIMEOUT_MS = 60000UL; // 60 seconds

// ============================================================
//  CPU frequency targets (MHz)
// ============================================================
constexpr int CPU_FREQ_FULL = 240;
constexpr int CPU_FREQ_LOW  = 80;

// ============================================================
//  Silence detection threshold for audio-interrupt wakeup
//  RMS value (0–32767 for 16-bit PCM).
//  Values below this are treated as silence.
// ============================================================
constexpr float SILENCE_RMS_THRESHOLD = 500.0f;

/**
 * PowerManager — Singleton
 *
 * Responsible for transitioning the system between three power states:
 *   FULL_ACTIVE     : 240 MHz, all servos active, mic streaming
 *   IDLE_LISTENING  : 240 MHz, tickless-idle enabled, servos active
 *   LOW_POWER_IDLE  : 80 MHz, all servos detached, mic in ISR-only wakeup mode
 *
 * FreeRTOS Tickless Idle (CONFIG_FREERTOS_USE_TICKLESS_IDLE) causes the
 * scheduler to automatically enter ESP32 Light Sleep whenever ALL tasks
 * are blocked on semaphores/queues — no explicit sleep call is needed.
 * Our semaphore-driven task design makes this happen naturally.
 *
 * The AudioISR (mic DMA interrupt) acts as the primary external hardware
 * interrupt to wake the system from LOW_POWER_IDLE — the ISR gives the
 * micWakeupSem semaphore when RMS exceeds SILENCE_RMS_THRESHOLD.
 */
class PowerManager {
public:
    static PowerManager& getInstance() {
        static PowerManager instance;
        return instance;
    }
    PowerManager(const PowerManager&) = delete;
    void operator=(const PowerManager&) = delete;

    /**
     * Call once during setup(). Configures ESP-IDF dynamic frequency
     * scaling so the CPU can drop to 80 MHz automatically via the PM driver.
     */
    void begin();

    /**
     * Transition to full power: 240 MHz CPU, all tasks unblocked.
     * Idempotent — safe to call if already in full power.
     */
    void enterFullPower();

    /**
     * Transition to low-power idle: 80 MHz CPU, all servos detached,
     * mic task switches to ISR-only wakeup mode.
     * Idempotent — safe to call if already in low-power.
     */
    void enterLowPowerIdle();

    bool isLowPower() const { return _isLowPower; }

    /**
     * Semaphore given by the Audio ISR when it detects audio above the
     * silence threshold. The audioUplinkTask blocks on this in low-power mode,
     * and uses it to trigger a system wakeup.
     * Created in begin(). Safe to give from ISR context.
     */
    SemaphoreHandle_t micWakeupSem = nullptr;

private:
    PowerManager() {}
    bool _isLowPower = false;
    void setCpuFrequency(int mhz);
};

#endif // POWER_MANAGER_H
