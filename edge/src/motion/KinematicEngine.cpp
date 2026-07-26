#include "motion/KinematicEngine.h"
#include "hardware/PCA9685_Driver.h"

KinematicEngine::KinematicEngine() {
    for (int i = 0; i < 16; i++) {
        states[i] = {0, 0, 0, 0, NONE, false};
        currentAngles[i] = 90.0; // safe default
    }
    // Set specific centers
    currentAngles[NECK_ONE.channel] = NECK_ONE.centerAngle;
    currentAngles[NECK_Y.channel] = NECK_Y.centerAngle;
    currentAngles[NECK_ROLL.channel] = NECK_ROLL.centerAngle;
    currentAngles[EYES_X.channel] = EYES_X.centerAngle;
    currentAngles[EYES_Y.channel] = EYES_Y.centerAngle;
    currentAngles[JAW_UD.channel] = JAW_UD.centerAngle;
    currentAngles[JAW_LR.channel] = JAW_LR.centerAngle;
    currentAngles[EYELID_LEFT.channel] = EYELID_LEFT.minAngle;
    currentAngles[EYELID_RIGHT.channel] = EYELID_RIGHT.minAngle;
}

void KinematicEngine::begin() {
    initDriver();
}

void KinematicEngine::triggerMove(const ServoConfig& config, double targetAngle, int durationMs, EasingType easingType) {
    uint8_t ch = config.channel;
    states[ch].startAngle = currentAngles[ch];
    states[ch].targetAngle = targetAngle;
    states[ch].startTimeMs = millis();
    states[ch].durationMs = durationMs;
    states[ch].easingType = easingType;
    states[ch].isMoving = true;
}

void KinematicEngine::updateKinematics() {
    uint32_t now = millis();
    
    // Pass 1: Calculate raw deltas and predicted current
    double rawDeltas[16] = {0};
    double peakCurrents[16] = {0};
    
    peakCurrents[NECK_ONE.channel] = 1.5;
    peakCurrents[NECK_Y.channel] = 1.5;
    peakCurrents[NECK_ROLL.channel] = 1.5;
    peakCurrents[JAW_UD.channel] = 1.0;
    peakCurrents[JAW_LR.channel] = 1.0;
    peakCurrents[EYES_X.channel] = 0.5;
    peakCurrents[EYES_Y.channel] = 0.5;
    peakCurrents[EYELID_LEFT.channel] = 0.5;
    peakCurrents[EYELID_RIGHT.channel] = 0.5;
    
    double totalPredictedCurrent = 0.0;
    
    for (int i = 0; i < 16; i++) {
        if (!states[i].isMoving) continue;
        
        uint32_t elapsed = now - states[i].startTimeMs;
        double currentAngle = states[i].startAngle;
        
        if (elapsed >= states[i].durationMs) {
            currentAngle = states[i].targetAngle;
        } else {
            double t = (double)elapsed / states[i].durationMs;
            double easedT = t;
            if (states[i].easingType == EASE_IN_OUT_SINE) easedT = easeInOutSine(t);
            else if (states[i].easingType == EASE_IN_OUT_CUBIC) easedT = easeInOutCubic(t);
            else if (states[i].easingType == EASE_OUT_EXPO) easedT = easeOutExpo(t);
            
            currentAngle = states[i].startAngle + ((states[i].targetAngle - states[i].startAngle) * easedT);
        }
        
        double delta = currentAngle > currentAngles[i] ? currentAngle - currentAngles[i] : currentAngles[i] - currentAngle;
        rawDeltas[i] = currentAngle - currentAngles[i];
        
        double currentDraw = peakCurrents[i];
        if (delta < 5.0) {
            currentDraw *= (delta / 5.0);
        }
        totalPredictedCurrent += currentDraw;
    }
    
    double scaleFactor = 1.0;
    if (totalPredictedCurrent > 8.0) {
        scaleFactor = 8.0 / totalPredictedCurrent;
    }
    
    // Pass 2: Apply scale factor and move
    for (int i = 0; i < 16; i++) {
        if (!states[i].isMoving) continue;
        
        double finalDelta = rawDeltas[i] * scaleFactor;
        double newAngle = currentAngles[i] + finalDelta;
        
        if (scaleFactor < 1.0) {
            // Compensate for lost time so the easing curve doesn't jump forward next frame
            // Assuming ~15ms task delay (from SystemTasks.cpp)
            uint32_t lostTime = (uint32_t)(15.0 * (1.0 - scaleFactor));
            states[i].startTimeMs += lostTime;
            states[i].durationMs += lostTime; // Also expand duration to preserve the curve shape
        }
        
        uint32_t elapsed = now - states[i].startTimeMs;
        if (elapsed >= states[i].durationMs && scaleFactor >= 0.99) {
            newAngle = states[i].targetAngle;
            states[i].isMoving = false;
        }
        
        // Retrieve config bounds
        double minA = 0, maxA = 180;
        if (i == NECK_ONE.channel) { minA = NECK_ONE.minAngle; maxA = NECK_ONE.maxAngle; }
        else if (i == NECK_Y.channel) { minA = NECK_Y.minAngle; maxA = NECK_Y.maxAngle; }
        else if (i == NECK_ROLL.channel) { minA = NECK_ROLL.minAngle; maxA = NECK_ROLL.maxAngle; }
        else if (i == EYES_X.channel) { minA = EYES_X.minAngle; maxA = EYES_X.maxAngle; }
        else if (i == EYES_Y.channel) { minA = EYES_Y.minAngle; maxA = EYES_Y.maxAngle; }
        else if (i == JAW_UD.channel) { minA = JAW_UD.minAngle; maxA = JAW_UD.maxAngle; }
        else if (i == JAW_LR.channel) { minA = JAW_LR.minAngle; maxA = JAW_LR.maxAngle; }
        else if (i == EYELID_LEFT.channel) { minA = EYELID_LEFT.minAngle; maxA = EYELID_LEFT.maxAngle; }
        else if (i == EYELID_RIGHT.channel) { minA = EYELID_RIGHT.minAngle; maxA = EYELID_RIGHT.maxAngle; }
        
        safeSetServoAngle(i, newAngle, minA, maxA);
        currentAngles[i] = newAngle;
    }
}
