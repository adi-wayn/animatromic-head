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
    
    // Process all moving servos
    for (int i = 0; i < 16; i++) {
        if (!states[i].isMoving) continue;
        
        double currentAngle = currentAngles[i];
        
        uint32_t elapsed = now - states[i].startTimeMs;
        if (elapsed >= states[i].durationMs) {
            currentAngle = states[i].targetAngle;
            states[i].isMoving = false;
        } else {
            double t = (double)elapsed / states[i].durationMs;
            double easedT = t;
            if (states[i].easingType == EASE_IN_OUT_SINE) easedT = easeInOutSine(t);
            else if (states[i].easingType == EASE_IN_OUT_CUBIC) easedT = easeInOutCubic(t);
            else if (states[i].easingType == EASE_OUT_EXPO) easedT = easeOutExpo(t);
            
            currentAngle = states[i].startAngle + ((states[i].targetAngle - states[i].startAngle) * easedT);
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
        
        safeSetServoAngle(i, currentAngle, minA, maxA);
        currentAngles[i] = currentAngle;
    }
}
