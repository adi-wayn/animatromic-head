#ifndef ANIMATRONIC_HEAD_H
#define ANIMATRONIC_HEAD_H

#include <Arduino.h>
#include "core/Config.h"

enum EasingType {
    EASE_IN_OUT_SINE, // Medium-speed conversational articulation
    EASE_IN_OUT_CUBIC, // Macro-movement posture shifts
    EASE_OUT_EXPO, // Rapid micro-movements (blinks/saccades)
    NONE
};

struct ServoState {
    double startAngle;
    double targetAngle;
    uint32_t startTimeMs;
    uint32_t durationMs;
    EasingType easingType;
    bool isMoving;
};

enum class SystemState {
    IDLE_LISTENING,
    SPEAKING_SYNCING,
    INTERRUPTED
};

class AnimatronicHead {
public:
    AnimatronicHead();

    void begin();

    // --- Basic Directions ---
    void lookLeft();
    void lookRight();
    void lookUp();
    void lookDown();
    void tiltRight();
    void tiltLeft();
    
    // --- Eyelids ---
    void blinkRight();
    void blinkLeft();
    void blink();

    // --- Jaw ---
    void jawOpen();
    void jawClose();
    void jawLeft();
    void jawRight();

    void expressHappy();
    void expressSad();
    void expressThinking(); // listenMode
    void resetToNeutral();
    
    // --- Pose Dictionary Execution ---
    // Maps string intents from JSON to servo sequences
    void executePose(const char* intent);
    
    // --- Idle Mode ---
    void toggleIdleMode();
    
    // --- Core Kinematic Loop ---
    // This MUST be called continuously (~60Hz) in the Core 1 FreeRTOS loop
    void updateKinematics();

    SystemState getState() const { return currentState; }
    void setState(SystemState newState) { currentState = newState; }

private:
    void triggerMove(const ServoConfig& config, double targetAngle, int durationMs, EasingType easingType);
    
    ServoState states[16]; // Indexed by channel (max 16 channels on PCA9685)
    double currentAngles[16];

    // --- Idle Mode State ---
    bool isIdleModeEnabled = false;
    
    SystemState currentState = SystemState::IDLE_LISTENING;
};

#endif // ANIMATRONIC_HEAD_H
