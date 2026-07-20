#ifndef ANIMATRONIC_HEAD_H
#define ANIMATRONIC_HEAD_H

#include <Arduino.h>

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

    // --- Emotions / Reactions ---
    void expressHappy();
    void expressSad();
    void expressThinking(); // listenMode
    void resetToNeutral();

private:
    // Helper to move a servo smoothly from its CURRENT angle to target
    void smoothMove(uint8_t channel, double targetAngle, double minLimit, double maxLimit, int durationMs);

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

    // Helper to get a reference to the tracked angle
    double& getTrackedAngle(uint8_t channel);
};

#endif // ANIMATRONIC_HEAD_H
