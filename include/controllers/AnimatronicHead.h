#ifndef ANIMATRONIC_HEAD_H
#define ANIMATRONIC_HEAD_H

#include <Arduino.h>
#include "core/Config.h"

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
    void updateIdleMicroMovements();

private:
    // Access the current tracker for a given channel
    double& getTrackedAngle(uint8_t channel);

    // Advanced movement functions
    void smoothMove(const ServoConfig& config, double targetAngle, int durationMs);
    void saccadeMove(const ServoConfig& config, double targetAngle, int durationMs);

    // Track current angles to prevent snapping
    double currentNeckOne;
    double currentNeckY;
    double currentNeckRoll;
    double currentEyesX;
    double currentEyesY;
    double currentJaw;     // Jaw Up/Down
    double currentJawLR;   // Jaw Left/Right
    double currentEyelidLeft;
    double currentEyelidRight;

    // --- Idle Mode State ---
    bool isIdleModeEnabled = false;
};

#endif // ANIMATRONIC_HEAD_H
