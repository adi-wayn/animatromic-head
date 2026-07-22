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
    void lookLeft(int durationMs = 500, EasingType easing = EASE_IN_OUT_SINE);
    void lookRight(int durationMs = 500, EasingType easing = EASE_IN_OUT_SINE);
    void lookUp(int durationMs = 500, EasingType easing = EASE_IN_OUT_SINE);
    void lookDown(int durationMs = 500, EasingType easing = EASE_IN_OUT_SINE);
    void tiltRight(int durationMs = 500, EasingType easing = EASE_IN_OUT_SINE);
    void tiltLeft(int durationMs = 500, EasingType easing = EASE_IN_OUT_SINE);
    
    // --- Eyelids ---
    void blinkRight(int durationMs = 150, EasingType easing = EASE_OUT_EXPO);
    void blinkLeft(int durationMs = 150, EasingType easing = EASE_OUT_EXPO);
    void blink(int durationMs = 150, EasingType easing = EASE_OUT_EXPO);

    // --- Jaw ---
    void jawOpen(int durationMs = 300, EasingType easing = EASE_IN_OUT_SINE);
    void jawClose(int durationMs = 300, EasingType easing = EASE_IN_OUT_SINE);
    void jawLeft(int durationMs = 300, EasingType easing = EASE_IN_OUT_SINE);
    void jawRight(int durationMs = 300, EasingType easing = EASE_IN_OUT_SINE);

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
