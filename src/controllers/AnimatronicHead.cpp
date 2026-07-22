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
                else if (states[i].easingType == EASE_IN_OUT_CUBIC) easedT = easeInOutCubic(t);
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

void AnimatronicHead::lookLeft(int durationMs, EasingType easing) {
    triggerMove(NECK_ONE, NECK_ONE.maxAngle, durationMs, easing);
    triggerMove(EYES_X, EYES_X.maxAngle, 150, EASE_OUT_EXPO);
}

void AnimatronicHead::lookRight(int durationMs, EasingType easing) {
    triggerMove(NECK_ONE, NECK_ONE.minAngle, durationMs, easing);
    triggerMove(EYES_X, EYES_X.minAngle, 150, EASE_OUT_EXPO);
}

void AnimatronicHead::lookUp(int durationMs, EasingType easing) {
    triggerMove(NECK_Y, NECK_Y.minAngle, durationMs, easing);
    triggerMove(EYES_Y, EYES_Y.minAngle, 150, EASE_OUT_EXPO); 
}

void AnimatronicHead::lookDown(int durationMs, EasingType easing) {
    triggerMove(NECK_Y, NECK_Y.maxAngle, durationMs, easing);
    triggerMove(EYES_Y, EYES_Y.maxAngle, 150, EASE_OUT_EXPO);
}

void AnimatronicHead::tiltRight(int durationMs, EasingType easing) {
    triggerMove(NECK_ROLL, NECK_ROLL.maxAngle, durationMs, easing);
}

void AnimatronicHead::tiltLeft(int durationMs, EasingType easing) {
    triggerMove(NECK_ROLL, NECK_ROLL.minAngle, durationMs, easing);
}

// --- Eyelids ---

void AnimatronicHead::blinkRight(int durationMs, EasingType easing) {
    // Hardware Defect Mitigation: mathematically slave the right eyelid to the left eyelid logic.
    // If told to blink right independently, we should just blink both to mask the defect.
    blink(durationMs, easing);
}

void AnimatronicHead::blinkLeft(int durationMs, EasingType easing) {
    blink(durationMs, easing);
}

void AnimatronicHead::blink(int durationMs, EasingType easing) {
    triggerMove(EYELID_LEFT, EYELID_LEFT.maxAngle, durationMs, easing);
    triggerMove(EYELID_RIGHT, EYELID_RIGHT.maxAngle, durationMs, easing);
}

// --- Jaw ---

void AnimatronicHead::jawOpen(int durationMs, EasingType easing) {
    triggerMove(JAW_UD, JAW_UD.minAngle, durationMs, easing);
}

void AnimatronicHead::jawClose(int durationMs, EasingType easing) {
    triggerMove(JAW_UD, JAW_UD.maxAngle, durationMs, easing);
}

void AnimatronicHead::jawLeft(int durationMs, EasingType easing) {
    // Hardware Defect Mitigation: clamp movement
    double target = JAW_LR.centerAngle - 10.0;
    if(target < JAW_LR.minAngle) target = JAW_LR.minAngle;
    triggerMove(JAW_LR, target, durationMs, easing);
}

void AnimatronicHead::jawRight(int durationMs, EasingType easing) {
    // Hardware Defect Mitigation: clamp movement
    double target = JAW_LR.centerAngle + 10.0;
    if(target > JAW_LR.maxAngle) target = JAW_LR.maxAngle;
    triggerMove(JAW_LR, target, durationMs, easing);
}

// --- Emotions / Reactions ---

void AnimatronicHead::executePose(const char* intent) {
    // Cognitive / Emotional Macros
    if (strcmp(intent, "HAPPY") == 0) expressHappy();
    else if (strcmp(intent, "SAD") == 0) expressSad();
    else if (strcmp(intent, "THINKING") == 0) expressThinking();
    else if (strcmp(intent, "NEUTRAL") == 0) resetToNeutral();
    
    // Direct Physical / Primitive Commands
    else if (strcmp(intent, "LOOK_LEFT") == 0) lookLeft();
    else if (strcmp(intent, "LOOK_RIGHT") == 0) lookRight();
    else if (strcmp(intent, "LOOK_UP") == 0) lookUp();
    else if (strcmp(intent, "LOOK_DOWN") == 0) lookDown();
    else if (strcmp(intent, "BLINK") == 0) blink();
    else if (strcmp(intent, "JAW_OPEN") == 0) jawOpen();
    else if (strcmp(intent, "JAW_CLOSE") == 0) jawClose();
    else resetToNeutral(); // Fallback
}

void AnimatronicHead::expressHappy() {
    // Happy is composed of a slight jaw open, a blink, and looking up, all executed simultaneously.
    // We override the default durations and easing to use Cubic for the macro posture change.
    blink(200, EASE_OUT_EXPO); 
    jawOpen(400, EASE_IN_OUT_SINE); 
    lookUp(400, EASE_IN_OUT_CUBIC);
}

void AnimatronicHead::expressSad() {
    double halfClosedLeft = (EYELID_LEFT.minAngle + EYELID_LEFT.maxAngle) / 2;
    double halfClosedRight = (EYELID_RIGHT.minAngle + EYELID_RIGHT.maxAngle) / 2;
    // Defect Mitigation: Eyelids slaved manually for partial angle
    triggerMove(EYELID_LEFT, halfClosedLeft, 250, EASE_OUT_EXPO);
    triggerMove(EYELID_RIGHT, halfClosedRight, 250, EASE_OUT_EXPO);
    
    // Compose with primitives
    jawClose(400, EASE_IN_OUT_SINE);
    lookDown(800, EASE_IN_OUT_CUBIC);

    // Partial pan (no primitive exists)
    triggerMove(NECK_ONE, NECK_ONE.centerAngle + 20, 800, EASE_IN_OUT_CUBIC);
}

void AnimatronicHead::expressThinking() {
    // Slaved Eyelids manually to center
    triggerMove(EYELID_LEFT, EYELID_LEFT.centerAngle, 200, EASE_OUT_EXPO);
    triggerMove(EYELID_RIGHT, EYELID_RIGHT.centerAngle, 200, EASE_OUT_EXPO);
    
    // Compose with jaw primitive
    jawClose(400, EASE_IN_OUT_SINE);

    // Macro movement (partial tilts, no primitives)
    triggerMove(NECK_ONE, NECK_ONE.centerAngle + 15, 600, EASE_IN_OUT_CUBIC);
    triggerMove(NECK_Y, NECK_Y.centerAngle - 10, 600, EASE_IN_OUT_CUBIC);
}

void AnimatronicHead::resetToNeutral() {
    // Macro movements (Neck & Head)
    triggerMove(NECK_ONE, NECK_ONE.centerAngle, 600, EASE_IN_OUT_CUBIC);
    triggerMove(NECK_Y, NECK_Y.centerAngle, 600, EASE_IN_OUT_CUBIC);
    triggerMove(NECK_ROLL, NECK_ROLL.centerAngle, 600, EASE_IN_OUT_CUBIC);
    
    // Micro movements (Eyes & Lids - Slaved)
    triggerMove(EYES_X, EYES_X.centerAngle, 300, EASE_OUT_EXPO);
    triggerMove(EYES_Y, EYES_Y.centerAngle, 300, EASE_OUT_EXPO);
    triggerMove(EYELID_LEFT, EYELID_LEFT.minAngle, 300, EASE_OUT_EXPO);
    triggerMove(EYELID_RIGHT, EYELID_RIGHT.minAngle, 300, EASE_OUT_EXPO);
    
    // Conversational/Jaw movements (Clamped)
    triggerMove(JAW_UD, JAW_UD.centerAngle, 500, EASE_IN_OUT_SINE);
    triggerMove(JAW_LR, JAW_LR.centerAngle, 500, EASE_IN_OUT_SINE);
}

void AnimatronicHead::toggleIdleMode() {
    isIdleModeEnabled = !isIdleModeEnabled;
}
