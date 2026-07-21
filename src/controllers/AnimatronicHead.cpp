#include "controllers/AnimatronicHead.h"
#include "hardware/PCA9685_Driver.h"
#include "motion/Easing.h"

AnimatronicHead::AnimatronicHead() {
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

void AnimatronicHead::begin() {
    initDriver();
}

void AnimatronicHead::triggerMove(const ServoConfig& config, double targetAngle, int durationMs, EasingType easingType) {
    uint8_t ch = config.channel;
    states[ch].startAngle = currentAngles[ch];
    states[ch].targetAngle = targetAngle;
    states[ch].startTimeMs = millis();
    states[ch].durationMs = durationMs;
    states[ch].easingType = easingType;
    states[ch].isMoving = true;
}

void AnimatronicHead::updateKinematics() {
    uint32_t now = millis();
    
    // First, calculate idle micro-movements if enabled, adding them as base offsets
    double noisePan = 0;
    double noiseTilt = 0;
    if (isIdleModeEnabled) {
        noisePan = generateSimpleNoise(now, 0.5f) * 10.0;
        noiseTilt = generateSimpleNoise(now, 0.35f) * 5.0;
    }

    // Process all moving servos
    for (int i = 0; i < 16; i++) {
        if (!states[i].isMoving && !isIdleModeEnabled) continue;
        
        double currentAngle = currentAngles[i];
        
        if (states[i].isMoving) {
            uint32_t elapsed = now - states[i].startTimeMs;
            if (elapsed >= states[i].durationMs) {
                currentAngle = states[i].targetAngle;
                states[i].isMoving = false;
            } else {
                double t = (double)elapsed / states[i].durationMs;
                double easedT = t;
                if (states[i].easingType == EASE_IN_OUT_SINE) easedT = easeInOutSine(t);
                else if (states[i].easingType == EASE_OUT_EXPO) easedT = easeOutExpo(t);
                
                currentAngle = states[i].startAngle + ((states[i].targetAngle - states[i].startAngle) * easedT);
            }
        }
        
        // Right Eyelid Mitigation: Mathematically slave right eyelid to left eyelid if they move together
        // We handle this at the command level below, but if there's drift, we can force it here.
        // Actually, we'll just let `currentAngle` calculate normally, but the command level ensures they match.
        
        // Apply idle noise offsets
        double outputAngle = currentAngle;
        if (isIdleModeEnabled && !states[i].isMoving) {
            if (i == NECK_ONE.channel) outputAngle = NECK_ONE.centerAngle + noisePan;
            if (i == NECK_Y.channel) outputAngle = NECK_Y.centerAngle + noiseTilt;
        }
        
        // Only write if there's a need to update
        if (states[i].isMoving || (isIdleModeEnabled && (i == NECK_ONE.channel || i == NECK_Y.channel))) {
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
            
            safeSetServoAngle(i, outputAngle, minA, maxA);
            currentAngles[i] = currentAngle;
        }
    }
}

// --- Basic Directions ---

void AnimatronicHead::lookLeft() {
    triggerMove(NECK_ONE, NECK_ONE.maxAngle, 500, EASE_IN_OUT_SINE);
    triggerMove(EYES_X, EYES_X.maxAngle, 150, EASE_OUT_EXPO);
}

void AnimatronicHead::lookRight() {
    triggerMove(NECK_ONE, NECK_ONE.minAngle, 500, EASE_IN_OUT_SINE);
    triggerMove(EYES_X, EYES_X.minAngle, 150, EASE_OUT_EXPO);
}

void AnimatronicHead::lookUp() {
    triggerMove(NECK_Y, NECK_Y.minAngle, 500, EASE_IN_OUT_SINE);
    triggerMove(EYES_Y, EYES_Y.minAngle, 150, EASE_OUT_EXPO); 
}

void AnimatronicHead::lookDown() {
    triggerMove(NECK_Y, NECK_Y.maxAngle, 500, EASE_IN_OUT_SINE);
    triggerMove(EYES_Y, EYES_Y.maxAngle, 150, EASE_OUT_EXPO);
}

void AnimatronicHead::tiltRight() {
    triggerMove(NECK_ROLL, NECK_ROLL.maxAngle, 500, EASE_IN_OUT_SINE);
}

void AnimatronicHead::tiltLeft() {
    triggerMove(NECK_ROLL, NECK_ROLL.minAngle, 500, EASE_IN_OUT_SINE);
}

// --- Eyelids ---

void AnimatronicHead::blinkRight() {
    // Hardware Defect Mitigation: mathematically slave the right eyelid to the left eyelid logic.
    // If told to blink right independently, we should just blink both to mask the defect.
    blink();
}

void AnimatronicHead::blinkLeft() {
    blink();
}

void AnimatronicHead::blink() {
    triggerMove(EYELID_LEFT, EYELID_LEFT.maxAngle, 150, EASE_OUT_EXPO);
    triggerMove(EYELID_RIGHT, EYELID_RIGHT.maxAngle, 150, EASE_OUT_EXPO);
}

// --- Jaw ---

void AnimatronicHead::jawOpen() {
    triggerMove(JAW_UD, JAW_UD.minAngle, 300, EASE_IN_OUT_SINE);
}

void AnimatronicHead::jawClose() {
    triggerMove(JAW_UD, JAW_UD.maxAngle, 300, EASE_IN_OUT_SINE);
}

void AnimatronicHead::jawLeft() {
    // Hardware Defect Mitigation: clamp movement
    double target = JAW_LR.centerAngle - 10.0;
    if(target < JAW_LR.minAngle) target = JAW_LR.minAngle;
    triggerMove(JAW_LR, target, 300, EASE_IN_OUT_SINE);
}

void AnimatronicHead::jawRight() {
    // Hardware Defect Mitigation: clamp movement
    double target = JAW_LR.centerAngle + 10.0;
    if(target > JAW_LR.maxAngle) target = JAW_LR.maxAngle;
    triggerMove(JAW_LR, target, 300, EASE_IN_OUT_SINE);
}

// --- Emotions / Reactions ---

void AnimatronicHead::expressHappy() {
    triggerMove(EYELID_LEFT, EYELID_LEFT.minAngle, 150, EASE_OUT_EXPO);
    triggerMove(EYELID_RIGHT, EYELID_RIGHT.minAngle, 150, EASE_OUT_EXPO);
    triggerMove(NECK_Y, NECK_Y.minAngle, 300, EASE_IN_OUT_SINE);
}

void AnimatronicHead::expressSad() {
    double halfClosedLeft = (EYELID_LEFT.minAngle + EYELID_LEFT.maxAngle) / 2;
    double halfClosedRight = (EYELID_RIGHT.minAngle + EYELID_RIGHT.maxAngle) / 2;
    triggerMove(EYELID_LEFT, halfClosedLeft, 150, EASE_OUT_EXPO);
    triggerMove(EYELID_RIGHT, halfClosedRight, 150, EASE_OUT_EXPO);
    triggerMove(NECK_Y, NECK_Y.maxAngle, 1000, EASE_IN_OUT_SINE);
}

void AnimatronicHead::expressThinking() {
    triggerMove(NECK_ONE, NECK_ONE.centerAngle + 15, 600, EASE_IN_OUT_SINE);
    triggerMove(NECK_Y, NECK_Y.centerAngle - 10, 600, EASE_IN_OUT_SINE);
}

void AnimatronicHead::resetToNeutral() {
    triggerMove(NECK_ONE, NECK_ONE.centerAngle, 500, EASE_IN_OUT_SINE);
    triggerMove(NECK_Y, NECK_Y.centerAngle, 500, EASE_IN_OUT_SINE);
    triggerMove(NECK_ROLL, NECK_ROLL.centerAngle, 500, EASE_IN_OUT_SINE);
    triggerMove(EYES_X, EYES_X.centerAngle, 300, EASE_OUT_EXPO);
    triggerMove(EYES_Y, EYES_Y.centerAngle, 300, EASE_OUT_EXPO);
    triggerMove(JAW_UD, JAW_UD.centerAngle, 500, EASE_IN_OUT_SINE);
    triggerMove(JAW_LR, JAW_LR.centerAngle, 500, EASE_IN_OUT_SINE);
    triggerMove(EYELID_LEFT, EYELID_LEFT.minAngle, 300, EASE_OUT_EXPO);
    triggerMove(EYELID_RIGHT, EYELID_RIGHT.minAngle, 300, EASE_OUT_EXPO);
}

void AnimatronicHead::toggleIdleMode() {
    isIdleModeEnabled = !isIdleModeEnabled;
}
