#ifndef ANIMATRONIC_HEAD_H
#define ANIMATRONIC_HEAD_H

#include "motion/KinematicEngine.h"
#include "controllers/PoseController.h"

enum class SystemState {
    IDLE_LISTENING,
    SPEAKING_SYNCING,
    INTERRUPTED
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

    // Facade Methods (Delegating to sub-systems)
    void begin() { KinematicEngine::getInstance().begin(); }
    void updateKinematics() { KinematicEngine::getInstance().updateKinematics(); }
    void executePose(const char* intent) { PoseController::getInstance().executePose(intent); }
    void triggerSaccade(uint32_t timeMs) { PoseController::getInstance().generateOrganicSaccade(timeMs); }

private:
    AnimatronicHead() {}
    bool fullyBooted = false;
    SystemState currentState = SystemState::IDLE_LISTENING;
};

#endif
