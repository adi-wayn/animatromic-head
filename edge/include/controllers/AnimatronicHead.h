#ifndef ANIMATRONIC_HEAD_H
#define ANIMATRONIC_HEAD_H

#include "motion/KinematicEngine.h"
#include "controllers/PoseController.h"

enum class SystemState {
    IDLE_LISTENING,     // Awake, listening for user speech
    SPEAKING_SYNCING,   // TTS playing, lip sync active
    INTERRUPTED,        // Emergency stop received mid-speech
    LOW_POWER_IDLE      // All servos detached, CPU throttled, mic in wakeup ISR mode
};

class AnimatronicHead {
public:
    static AnimatronicHead& getInstance() {
        static AnimatronicHead instance;
        return instance;
    }
    AnimatronicHead(const AnimatronicHead&) = delete;
    void operator=(const AnimatronicHead&) = delete;

    // State Management
    SystemState getState() const { return currentState; }
    void setState(SystemState newState) { currentState = newState; }
    bool isBooted() const { return fullyBooted; }
    void setBooted(bool state) { fullyBooted = state; }
    bool isInLowPowerIdle() const { return currentState == SystemState::LOW_POWER_IDLE; }

    // Inactivity tracking — updated on any host packet, speech event, or wakeup
    void updateActivityTimestamp() { _lastActivityMs = millis(); }
    uint32_t getLastActivityMs() const { return _lastActivityMs; }

    // Facade Methods (Delegating to sub-systems)
    void begin() { KinematicEngine::getInstance().begin(); }
    void updateKinematics() { KinematicEngine::getInstance().updateKinematics(); }
    void executePose(const char* intent) { PoseController::getInstance().executePose(intent); }
    void triggerSaccade(uint32_t timeMs) { PoseController::getInstance().generateOrganicSaccade(timeMs); }

private:
    AnimatronicHead() {}
    bool fullyBooted = false;
    SystemState currentState = SystemState::IDLE_LISTENING;
    uint32_t _lastActivityMs = 0;
};

#endif

