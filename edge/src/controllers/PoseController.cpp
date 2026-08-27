#include "controllers/PoseController.h"
#include "motion/KinematicEngine.h"
#include <string.h>

void PoseController::executePose(const char* intent) {
    // Cognitive / Emotional Macros
    if (strcmp(intent, "HAPPY") == 0) expressHappy();
    else if (strcmp(intent, "SAD") == 0) expressSad();
    else if (strcmp(intent, "THINKING") == 0) expressThinking();
    else if (strcmp(intent, "ANGRY") == 0) expressAngry();
    else if (strcmp(intent, "SURPRISED") == 0) expressSurprised();
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

// --- Strict Atomic Building Blocks ---

void PoseController::moveNeckPan(double angle, int durationMs, EasingType easing) {
    KinematicEngine::getInstance().triggerMove(NECK_ONE, angle, durationMs, easing);
}
void PoseController::moveNeckTilt(double angle, int durationMs, EasingType easing) {
    KinematicEngine::getInstance().triggerMove(NECK_Y, angle, durationMs, easing);
}
void PoseController::moveNeckRoll(double angle, int durationMs, EasingType easing) {
    KinematicEngine::getInstance().triggerMove(NECK_ROLL, angle, durationMs, easing);
}
void PoseController::moveEyesPan(double angle, int durationMs, EasingType easing) {
    KinematicEngine::getInstance().triggerMove(EYES_X, angle, durationMs, easing);
}
void PoseController::moveEyesTilt(double angle, int durationMs, EasingType easing) {
    KinematicEngine::getInstance().triggerMove(EYES_Y, angle, durationMs, easing);
}
void PoseController::moveEyelidLeft(double angle, int durationMs, EasingType easing) {
    KinematicEngine::getInstance().triggerMove(EYELID_LEFT, angle, durationMs, easing);
}
void PoseController::moveEyelidRight(double angle, int durationMs, EasingType easing) {
    KinematicEngine::getInstance().triggerMove(EYELID_RIGHT, angle, durationMs, easing);
}
void PoseController::moveJawUD(double angle, int durationMs, EasingType easing) {
    KinematicEngine::getInstance().triggerMove(JAW_UD, angle, durationMs, easing);
}
void PoseController::moveJawLR(double targetAngle, int durationMs, EasingType easing) {
    // Hardware Defect Mitigation: strict clamping applied at the atomic level
    if(targetAngle < JAW_LR.minAngle) targetAngle = JAW_LR.minAngle;
    if(targetAngle > JAW_LR.maxAngle) targetAngle = JAW_LR.maxAngle;
    KinematicEngine::getInstance().triggerMove(JAW_LR, targetAngle, durationMs, easing);
}

void PoseController::generateOrganicSaccade(uint32_t timeMs) {
    // Generate organic noise [-1.0, 1.0] using the existing wave superposition
    double noiseX = generateSimpleNoise(timeMs, 1.2f); 
    double noiseY = generateSimpleNoise(timeMs + 1000, 0.8f);

    double xRange = (EYES_X.maxAngle - EYES_X.minAngle) * 0.20; // 20% physical range for larger X twitches
    double yRange = (EYES_Y.maxAngle - EYES_Y.minAngle) * 0.15; // 15% physical range for more natural up/down movement

    double targetX = EYES_X.centerAngle + (noiseX * xRange);
    double targetY = EYES_Y.centerAngle + (noiseY * yRange);

    moveEyesPan(targetX, 800, EASE_IN_OUT_SINE);
    moveEyesTilt(targetY, 800, EASE_IN_OUT_SINE);
    
    // Add organic neck drift (slow and visible)
    // The previous range was too small (3-5 degrees) to overcome servo deadband.
    // generateSimpleNoise maxes out around 0.3-0.4, so we need a larger multiplier.
    double neckNoiseX = generateSimpleNoise(timeMs + 5000, 0.4f) * 2.5; 
    double neckNoiseY = generateSimpleNoise(timeMs + 7000, 0.3f) * 2.5;
    
    // Clamp noise to roughly [-1.0, 1.0]
    if(neckNoiseX > 1.0) neckNoiseX = 1.0; else if(neckNoiseX < -1.0) neckNoiseX = -1.0;
    if(neckNoiseY > 1.0) neckNoiseY = 1.0; else if(neckNoiseY < -1.0) neckNoiseY = -1.0;
    
    double neckXRange = (NECK_ONE.maxAngle - NECK_ONE.minAngle) * 0.25; // 25% physical range
    double neckYRange = (NECK_Y.maxAngle - NECK_Y.minAngle) * 0.20; // 20% physical range
    
    double targetNeckX = NECK_ONE.centerAngle + (neckNoiseX * neckXRange);
    double targetNeckY = NECK_Y.centerAngle + (neckNoiseY * neckYRange);
    
    // Since idle task runs at random intervals between 800-2500ms, a 1500ms ease looks smooth
    moveNeckPan(targetNeckX, 1500, EASE_IN_OUT_SINE);
    moveNeckTilt(targetNeckY, 1500, EASE_IN_OUT_SINE);
}

void PoseController::syncJawToAmplitude(float intensity) {
    // MAPPING: maxAngle = CLOSED (High angle), minAngle = OPEN (Low angle)
    double rangeUD = JAW_UD.maxAngle - JAW_UD.minAngle;
    double targetUD = JAW_UD.maxAngle - (intensity * rangeUD);
    
    // Setting duration to 0 is CRITICAL because this function is called continuously.
    // If >0, the KinematicEngine resets the start time every frame and the servo never moves!
    moveJawUD(targetUD, 0, EASE_IN_OUT_SINE);
    
    // 2. Secondary axis (Organic texture - LR)
    // Blend a tiny bit of left/right motion based on intensity for realism
    double targetLR = JAW_LR.centerAngle + ((intensity * 10.0) - 5.0); // +/- 5 degrees
    moveJawLR(targetLR, 0, EASE_IN_OUT_SINE);
}

// --- Basic Directions ---

void PoseController::lookLeft(int durationMs, EasingType easing) {
    moveNeckPan(NECK_ONE.maxAngle, durationMs, easing);
    moveEyesPan(EYES_X.maxAngle, durationMs, EASE_IN_OUT_CUBIC);
}

void PoseController::lookRight(int durationMs, EasingType easing) {
    moveNeckPan(NECK_ONE.minAngle, durationMs, easing);
    moveEyesPan(EYES_X.minAngle, durationMs, EASE_IN_OUT_CUBIC);
}

void PoseController::lookUp(int durationMs, EasingType easing) {
    moveNeckTilt(NECK_Y.minAngle, durationMs, easing);
    moveEyesTilt(EYES_Y.minAngle, durationMs, EASE_IN_OUT_CUBIC); 
}

void PoseController::lookDown(int durationMs, EasingType easing) {
    moveNeckTilt(NECK_Y.maxAngle, durationMs, easing);
    moveEyesTilt(EYES_Y.maxAngle, durationMs, EASE_IN_OUT_CUBIC);
}

void PoseController::tiltRight(int durationMs, EasingType easing) {
    moveNeckRoll(NECK_ROLL.maxAngle, durationMs, easing);
}

void PoseController::tiltLeft(int durationMs, EasingType easing) {
    moveNeckRoll(NECK_ROLL.minAngle, durationMs, easing);
}

// --- Eyelids ---

void PoseController::blinkRight(int durationMs, EasingType easing) {
    blink(durationMs, easing);
}

void PoseController::blinkLeft(int durationMs, EasingType easing) {
    blink(durationMs, easing);
}

void PoseController::blink(int durationMs, EasingType easing) {
    // 1. Close eyelids (150ms)
    moveEyelidLeft(EYELID_LEFT.maxAngle, 150, easing);
    moveEyelidRight(EYELID_RIGHT.maxAngle, 150, easing);
    
    // 2. Schedule re-opening 150ms later (using simple delay block for atomic primitive)
    vTaskDelay(pdMS_TO_TICKS(150));
    
    // 3. Open eyelids (slower, 200ms)
    moveEyelidLeft(EYELID_LEFT.minAngle, 200, EASE_OUT_EXPO);
    moveEyelidRight(EYELID_RIGHT.minAngle, 200, EASE_OUT_EXPO);
}

// --- Jaw ---

void PoseController::jawOpen(int durationMs, EasingType easing) {
    moveJawUD(JAW_UD.minAngle, durationMs, easing);
}

void PoseController::jawClose(int durationMs, EasingType easing) {
    moveJawUD(JAW_UD.maxAngle, durationMs, easing);
}

void PoseController::jawLeft(int durationMs, EasingType easing) {
    moveJawLR(JAW_LR.centerAngle - 10.0, durationMs, easing);
}

void PoseController::jawRight(int durationMs, EasingType easing) {
    moveJawLR(JAW_LR.centerAngle + 10.0, durationMs, easing);
}

// --- Emotions / Reactions ---

void PoseController::expressHappy() {
    blink(200, EASE_OUT_EXPO); 
    jawOpen(400, EASE_IN_OUT_SINE); 
    lookUp(400, EASE_IN_OUT_CUBIC);
}

void PoseController::expressSad() {
    double halfClosedLeft = (EYELID_LEFT.minAngle + EYELID_LEFT.maxAngle) / 2;
    double halfClosedRight = (EYELID_RIGHT.minAngle + EYELID_RIGHT.maxAngle) / 2;
    moveEyelidLeft(halfClosedLeft, 250, EASE_OUT_EXPO);
    moveEyelidRight(halfClosedRight, 250, EASE_OUT_EXPO);
    
    jawClose(400, EASE_IN_OUT_SINE);
    moveNeckTilt(NECK_Y.centerAngle + 8.0, 800, EASE_IN_OUT_CUBIC);
    moveNeckPan(NECK_ONE.centerAngle + 20, 800, EASE_IN_OUT_CUBIC);
}

void PoseController::expressThinking() {
    moveEyelidLeft(EYELID_LEFT.centerAngle, 200, EASE_OUT_EXPO);
    moveEyelidRight(EYELID_RIGHT.centerAngle, 200, EASE_OUT_EXPO);
    
    lookUp(600, EASE_IN_OUT_CUBIC);
    moveNeckRoll(NECK_ROLL.centerAngle + 15, 500, EASE_IN_OUT_CUBIC);
    jawClose(200, EASE_IN_OUT_SINE);
}

void PoseController::expressAngry() {
    // Eyelids lowered aggressively
    moveEyelidLeft(EYELID_LEFT.centerAngle + 20, 150, EASE_OUT_EXPO);
    moveEyelidRight(EYELID_RIGHT.centerAngle + 20, 150, EASE_OUT_EXPO);
    
    // Head snaps forward and down slightly (less extreme)
    moveNeckPan(NECK_ONE.centerAngle, 300, EASE_IN_OUT_CUBIC);
    moveNeckTilt(NECK_Y.centerAngle + 5.0, 300, EASE_IN_OUT_CUBIC);
    
    // Jaw clenches
    jawClose(150, EASE_IN_OUT_SINE);
}

void PoseController::expressSurprised() {
    // Eyes wide open
    moveEyelidLeft(EYELID_LEFT.minAngle, 150, EASE_OUT_EXPO);
    moveEyelidRight(EYELID_RIGHT.minAngle, 150, EASE_OUT_EXPO);
    
    // Head pulls back and up
    moveNeckPan(NECK_ONE.centerAngle, 200, EASE_IN_OUT_CUBIC);
    lookUp(200, EASE_IN_OUT_CUBIC);
    
    // Jaw drops
    jawOpen(200, EASE_IN_OUT_SINE);
}

void PoseController::resetToNeutral() {
    moveNeckPan(NECK_ONE.centerAngle, 600, EASE_IN_OUT_CUBIC);
    moveNeckTilt(NECK_Y.centerAngle, 600, EASE_IN_OUT_CUBIC); 
    moveNeckRoll(NECK_ROLL.centerAngle, 600, EASE_IN_OUT_CUBIC); 
    
    moveEyesPan(EYES_X.centerAngle, 600, EASE_IN_OUT_CUBIC);
    moveEyesTilt(EYES_Y.centerAngle, 600, EASE_IN_OUT_CUBIC);
    moveEyelidLeft(EYELID_LEFT.minAngle, 300, EASE_OUT_EXPO);
    moveEyelidRight(EYELID_RIGHT.minAngle, 300, EASE_OUT_EXPO);
    
    moveJawUD(JAW_UD.maxAngle, 500, EASE_IN_OUT_SINE);
    moveJawLR(JAW_LR.centerAngle, 500, EASE_IN_OUT_SINE);
}
