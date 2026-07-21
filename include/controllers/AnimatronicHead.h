#ifndef ANIMATRONIC_HEAD_H
#define ANIMATRONIC_HEAD_H

#include <Arduino.h>
#include "core/Config.h"

enum EasingType {
    EASE_IN_OUT_SINE,
    EASE_OUT_EXPO,
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
    
    // --- Idle Mode ---
    void toggleIdleMode();
    
    // --- Core Kinematic Loop ---
    // This MUST be called continuously (~60Hz) in the Core 1 FreeRTOS loop
    void updateKinematics();

private:
    void triggerMove(const ServoConfig& config, double targetAngle, int durationMs, EasingType easingType);
    
    ServoState states[16]; // Indexed by channel (max 16 channels on PCA9685)
    double currentAngles[16];

    // --- Idle Mode State ---
    bool isIdleModeEnabled = false;
};

#endif // ANIMATRONIC_HEAD_H
