#include "controllers/PoseController.h"
#include "motion/KinematicEngine.h"
#include <string.h>

void PoseController::executePose(const char* intent) {
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

void PoseController::generateOrganicSaccade(uint32_t timeMs) {
    // Generate organic noise [-1.0, 1.0] using the existing wave superposition
    double noiseX = generateSimpleNoise(timeMs, 1.2f); 
    double noiseY = generateSimpleNoise(timeMs + 1000, 0.8f);

    double xRange = (EYES_X.maxAngle - EYES_X.minAngle) * 0.15; // 15% physical range
    double yRange = (EYES_Y.maxAngle - EYES_Y.minAngle) * 0.15;

    double targetX = EYES_X.centerAngle + (noiseX * xRange);
    double targetY = EYES_Y.centerAngle + (noiseY * yRange);

    KinematicEngine::getInstance().triggerMove(EYES_X, targetX, 100, EASE_OUT_EXPO);
    KinematicEngine::getInstance().triggerMove(EYES_Y, targetY, 100, EASE_OUT_EXPO);
}

// --- Basic Directions ---

void PoseController::lookLeft(int durationMs, EasingType easing) {
    KinematicEngine::getInstance().triggerMove(NECK_ONE, NECK_ONE.maxAngle, durationMs, easing);
    KinematicEngine::getInstance().triggerMove(EYES_X, EYES_X.maxAngle, 150, EASE_OUT_EXPO);
}

void PoseController::lookRight(int durationMs, EasingType easing) {
    KinematicEngine::getInstance().triggerMove(NECK_ONE, NECK_ONE.minAngle, durationMs, easing);
    KinematicEngine::getInstance().triggerMove(EYES_X, EYES_X.minAngle, 150, EASE_OUT_EXPO);
}

void PoseController::lookUp(int durationMs, EasingType easing) {
    KinematicEngine::getInstance().triggerMove(NECK_Y, NECK_Y.minAngle, durationMs, easing);
    KinematicEngine::getInstance().triggerMove(EYES_Y, EYES_Y.minAngle, 150, EASE_OUT_EXPO); 
}

void PoseController::lookDown(int durationMs, EasingType easing) {
    KinematicEngine::getInstance().triggerMove(NECK_Y, NECK_Y.maxAngle, durationMs, easing);
    KinematicEngine::getInstance().triggerMove(EYES_Y, EYES_Y.maxAngle, 150, EASE_OUT_EXPO);
}

void PoseController::tiltRight(int durationMs, EasingType easing) {
    KinematicEngine::getInstance().triggerMove(NECK_ROLL, NECK_ROLL.maxAngle, durationMs, easing);
}

void PoseController::tiltLeft(int durationMs, EasingType easing) {
    KinematicEngine::getInstance().triggerMove(NECK_ROLL, NECK_ROLL.minAngle, durationMs, easing);
}

// --- Eyelids ---

void PoseController::blinkRight(int durationMs, EasingType easing) {
    blink(durationMs, easing);
}

void PoseController::blinkLeft(int durationMs, EasingType easing) {
    blink(durationMs, easing);
}

void PoseController::blink(int durationMs, EasingType easing) {
    KinematicEngine::getInstance().triggerMove(EYELID_LEFT, EYELID_LEFT.maxAngle, durationMs, easing);
    KinematicEngine::getInstance().triggerMove(EYELID_RIGHT, EYELID_RIGHT.maxAngle, durationMs, easing);
}

// --- Jaw ---

void PoseController::jawOpen(int durationMs, EasingType easing) {
    KinematicEngine::getInstance().triggerMove(JAW_UD, JAW_UD.minAngle, durationMs, easing);
}

void PoseController::jawClose(int durationMs, EasingType easing) {
    KinematicEngine::getInstance().triggerMove(JAW_UD, JAW_UD.maxAngle, durationMs, easing);
}

void PoseController::jawLeft(int durationMs, EasingType easing) {
    double target = JAW_LR.centerAngle - 10.0;
    if(target < JAW_LR.minAngle) target = JAW_LR.minAngle;
    KinematicEngine::getInstance().triggerMove(JAW_LR, target, durationMs, easing);
}

void PoseController::jawRight(int durationMs, EasingType easing) {
    double target = JAW_LR.centerAngle + 10.0;
    if(target > JAW_LR.maxAngle) target = JAW_LR.maxAngle;
    KinematicEngine::getInstance().triggerMove(JAW_LR, target, durationMs, easing);
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
    KinematicEngine::getInstance().triggerMove(EYELID_LEFT, halfClosedLeft, 250, EASE_OUT_EXPO);
    KinematicEngine::getInstance().triggerMove(EYELID_RIGHT, halfClosedRight, 250, EASE_OUT_EXPO);
    
    jawClose(400, EASE_IN_OUT_SINE);
    lookDown(800, EASE_IN_OUT_CUBIC);
    KinematicEngine::getInstance().triggerMove(NECK_ONE, NECK_ONE.centerAngle + 20, 800, EASE_IN_OUT_CUBIC);
}

void PoseController::expressThinking() {
    KinematicEngine::getInstance().triggerMove(EYELID_LEFT, EYELID_LEFT.centerAngle, 200, EASE_OUT_EXPO);
    KinematicEngine::getInstance().triggerMove(EYELID_RIGHT, EYELID_RIGHT.centerAngle, 200, EASE_OUT_EXPO);
    
    jawClose(400, EASE_IN_OUT_SINE);
    KinematicEngine::getInstance().triggerMove(NECK_ONE, NECK_ONE.centerAngle + 15, 600, EASE_IN_OUT_CUBIC);
    KinematicEngine::getInstance().triggerMove(NECK_Y, NECK_Y.centerAngle - 10, 600, EASE_IN_OUT_CUBIC);
}

void PoseController::resetToNeutral() {
    KinematicEngine::getInstance().triggerMove(NECK_ONE, NECK_ONE.centerAngle, 600, EASE_IN_OUT_CUBIC);
    KinematicEngine::getInstance().triggerMove(NECK_Y, NECK_Y.centerAngle, 600, EASE_IN_OUT_CUBIC);
    KinematicEngine::getInstance().triggerMove(NECK_ROLL, NECK_ROLL.centerAngle, 600, EASE_IN_OUT_CUBIC);
    
    KinematicEngine::getInstance().triggerMove(EYES_X, EYES_X.centerAngle, 300, EASE_OUT_EXPO);
    KinematicEngine::getInstance().triggerMove(EYES_Y, EYES_Y.centerAngle, 300, EASE_OUT_EXPO);
    KinematicEngine::getInstance().triggerMove(EYELID_LEFT, EYELID_LEFT.minAngle, 300, EASE_OUT_EXPO);
    KinematicEngine::getInstance().triggerMove(EYELID_RIGHT, EYELID_RIGHT.minAngle, 300, EASE_OUT_EXPO);
    
    KinematicEngine::getInstance().triggerMove(JAW_UD, JAW_UD.centerAngle, 500, EASE_IN_OUT_SINE);
    KinematicEngine::getInstance().triggerMove(JAW_LR, JAW_LR.centerAngle, 500, EASE_IN_OUT_SINE);
}
