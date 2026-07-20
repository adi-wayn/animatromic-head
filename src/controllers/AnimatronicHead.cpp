#include "controllers/AnimatronicHead.h"
#include "hardware/PCA9685_Driver.h"
#include "motion/Easing.h"

AnimatronicHead::AnimatronicHead() {
    // Initialize trackers to center/default positions
    currentNeckOne = NECK_ONE.centerAngle;
    currentNeckY = NECK_Y.centerAngle;
    currentNeckRoll = NECK_ROLL.centerAngle;
    currentEyesX = EYES_X.centerAngle;
    currentEyesY = EYES_Y.centerAngle;
    currentJaw = JAW_UD.centerAngle;
    currentJawLR = JAW_LR.centerAngle;
    currentEyelidLeft = EYELID_LEFT.minAngle;
    currentEyelidRight = EYELID_RIGHT.minAngle;
}

void AnimatronicHead::begin() {
    initDriver();
    // Intentionally NOT sending any servo commands here. 
    // This ensures a "silent boot" to prevent power spikes or immediate crazy movements.
    // The servos will only move when you press a key in the CLI.
}

double& AnimatronicHead::getTrackedAngle(uint8_t channel) {
    if (channel == NECK_ONE.channel) return currentNeckOne;
    if (channel == NECK_Y.channel) return currentNeckY;
    if (channel == NECK_ROLL.channel) return currentNeckRoll;
    if (channel == EYES_X.channel) return currentEyesX;
    if (channel == EYES_Y.channel) return currentEyesY;
    if (channel == JAW_UD.channel) return currentJaw;
    if (channel == JAW_LR.channel) return currentJawLR;
    if (channel == EYELID_LEFT.channel) return currentEyelidLeft;
    if (channel == EYELID_RIGHT.channel) return currentEyelidRight;
    return currentNeckOne; // fallback
}

void AnimatronicHead::smoothMove(const ServoConfig& config, double targetAngle, int durationMs) {
    double& startAngle = getTrackedAngle(config.channel);
    int steps = durationMs / 15; // 15ms per step for smooth 60fps movement
    if (steps < 1) steps = 1;
    
    for (int i = 1; i <= steps; i++) {
        double t = (double)i / steps;
        double easedT = easeInOutSine(t);
        double currentAngle = startAngle + ((targetAngle - startAngle) * easedT);
        safeSetServoAngle(config.channel, currentAngle, config.minAngle, config.maxAngle);
        vTaskDelay(pdMS_TO_TICKS(15));
    }
    startAngle = targetAngle; // Update tracker
}

void AnimatronicHead::saccadeMove(const ServoConfig& config, double targetAngle, int durationMs) {
    double& startAngle = getTrackedAngle(config.channel);
    int steps = durationMs / 15;
    if (steps < 1) steps = 1;
    
    for (int i = 1; i <= steps; i++) {
        double t = (double)i / steps;
        double easedT = easeOutExpo(t);
        double currentAngle = startAngle + ((targetAngle - startAngle) * easedT);
        safeSetServoAngle(config.channel, currentAngle, config.minAngle, config.maxAngle);
        vTaskDelay(pdMS_TO_TICKS(15));
    }
    startAngle = targetAngle; // Update tracker
}

// --- Basic Directions ---

void AnimatronicHead::lookLeft() {
    smoothMove(NECK_ONE, NECK_ONE.maxAngle, 500);
    saccadeMove(EYES_X, EYES_X.maxAngle, 150);
}

void AnimatronicHead::lookRight() {
    smoothMove(NECK_ONE, NECK_ONE.minAngle, 500);
    saccadeMove(EYES_X, EYES_X.minAngle, 150);
}

void AnimatronicHead::lookUp() {
    smoothMove(NECK_Y, NECK_Y.minAngle, 500); // Inverted: Min angle goes UP
    saccadeMove(EYES_Y, EYES_Y.minAngle, 150); 
}

void AnimatronicHead::lookDown() {
    smoothMove(NECK_Y, NECK_Y.maxAngle, 500); // Inverted: Max angle goes DOWN
    saccadeMove(EYES_Y, EYES_Y.maxAngle, 150);
}

void AnimatronicHead::tiltRight() {
    smoothMove(NECK_ROLL, NECK_ROLL.maxAngle, 500);
}

void AnimatronicHead::tiltLeft() {
    smoothMove(NECK_ROLL, NECK_ROLL.minAngle, 500);
}

// --- Eyelids ---

void AnimatronicHead::blinkRight() {
    safeSetServoAngle(EYELID_RIGHT.channel, EYELID_RIGHT.maxAngle, EYELID_RIGHT.minAngle, EYELID_RIGHT.maxAngle);
    vTaskDelay(pdMS_TO_TICKS(150));
    safeSetServoAngle(EYELID_RIGHT.channel, EYELID_RIGHT.minAngle, EYELID_RIGHT.minAngle, EYELID_RIGHT.maxAngle);
    currentEyelidRight = EYELID_RIGHT.minAngle;
}

void AnimatronicHead::blinkLeft() {
    safeSetServoAngle(EYELID_LEFT.channel, EYELID_LEFT.maxAngle, EYELID_LEFT.minAngle, EYELID_LEFT.maxAngle);
    vTaskDelay(pdMS_TO_TICKS(150));
    safeSetServoAngle(EYELID_LEFT.channel, EYELID_LEFT.minAngle, EYELID_LEFT.minAngle, EYELID_LEFT.maxAngle);
    currentEyelidLeft = EYELID_LEFT.minAngle;
}

void AnimatronicHead::blink() {
    safeSetServoAngle(EYELID_RIGHT.channel, EYELID_RIGHT.maxAngle, EYELID_RIGHT.minAngle, EYELID_RIGHT.maxAngle);
    safeSetServoAngle(EYELID_LEFT.channel, EYELID_LEFT.maxAngle, EYELID_LEFT.minAngle, EYELID_LEFT.maxAngle);
    vTaskDelay(pdMS_TO_TICKS(150));
    safeSetServoAngle(EYELID_RIGHT.channel, EYELID_RIGHT.minAngle, EYELID_RIGHT.minAngle, EYELID_RIGHT.maxAngle);
    safeSetServoAngle(EYELID_LEFT.channel, EYELID_LEFT.minAngle, EYELID_LEFT.minAngle, EYELID_LEFT.maxAngle);
    currentEyelidRight = EYELID_RIGHT.minAngle;
    currentEyelidLeft = EYELID_LEFT.minAngle;
}

// --- Jaw ---

void AnimatronicHead::jawOpen() {
    smoothMove(JAW_UD, JAW_UD.minAngle, 300);
}

void AnimatronicHead::jawClose() {
    smoothMove(JAW_UD, JAW_UD.maxAngle, 300);
}

void AnimatronicHead::jawLeft() {
    smoothMove(JAW_LR, JAW_LR.minAngle, 300);
}

void AnimatronicHead::jawRight() {
    smoothMove(JAW_LR, JAW_LR.maxAngle, 300);
}

// --- Emotions / Reactions ---

void AnimatronicHead::expressHappy() {
    safeSetServoAngle(EYELID_LEFT.channel, EYELID_LEFT.minAngle, EYELID_LEFT.minAngle, EYELID_LEFT.maxAngle);
    safeSetServoAngle(EYELID_RIGHT.channel, EYELID_RIGHT.minAngle, EYELID_RIGHT.minAngle, EYELID_RIGHT.maxAngle);
    currentEyelidLeft = EYELID_LEFT.minAngle;
    currentEyelidRight = EYELID_RIGHT.minAngle;
    smoothMove(NECK_Y, NECK_Y.minAngle, 300); // Look UP
}

void AnimatronicHead::expressSad() {
    double halfClosedLeft = (EYELID_LEFT.minAngle + EYELID_LEFT.maxAngle) / 2;
    double halfClosedRight = (EYELID_RIGHT.minAngle + EYELID_RIGHT.maxAngle) / 2;
    safeSetServoAngle(EYELID_LEFT.channel, halfClosedLeft, EYELID_LEFT.minAngle, EYELID_LEFT.maxAngle);
    safeSetServoAngle(EYELID_RIGHT.channel, halfClosedRight, EYELID_RIGHT.minAngle, EYELID_RIGHT.maxAngle);
    currentEyelidLeft = halfClosedLeft;
    currentEyelidRight = halfClosedRight;
    smoothMove(NECK_Y, NECK_Y.maxAngle, 1000); // Look DOWN
}

void AnimatronicHead::expressThinking() {
    smoothMove(NECK_ONE, NECK_ONE.centerAngle + 15, 600);
    smoothMove(NECK_Y, NECK_Y.centerAngle - 10, 600); // Inverted tilt
}

void AnimatronicHead::resetToNeutral() {
    smoothMove(NECK_ONE, NECK_ONE.centerAngle, 500);
    smoothMove(NECK_Y, NECK_Y.centerAngle, 500);
    smoothMove(NECK_ROLL, NECK_ROLL.centerAngle, 500);
    saccadeMove(EYES_X, EYES_X.centerAngle, 300);
    saccadeMove(EYES_Y, EYES_Y.centerAngle, 300);
    smoothMove(JAW_UD, JAW_UD.centerAngle, 500);
    smoothMove(JAW_LR, JAW_LR.centerAngle, 500);
    smoothMove(EYELID_LEFT, EYELID_LEFT.minAngle, 300);
    smoothMove(EYELID_RIGHT, EYELID_RIGHT.minAngle, 300);
}

// --- Idle Mode ---

void AnimatronicHead::toggleIdleMode() {
    isIdleModeEnabled = !isIdleModeEnabled;
}

void AnimatronicHead::updateIdleMicroMovements() {
    if (!isIdleModeEnabled) return;
    
    uint32_t now = millis();
    
    // Perlin-style noise for neck pan (Left/Right)
    // Low frequency for slow swaying
    double noisePan = generateSimpleNoise(now, 0.5f);
    double targetPan = NECK_ONE.centerAngle + (noisePan * 10.0); // +/- 10 degrees sway
    
    // Perlin-style noise for neck tilt (Up/Down), slightly different frequency to de-sync
    double noiseTilt = generateSimpleNoise(now, 0.35f);
    double targetTilt = NECK_Y.centerAngle + (noiseTilt * 5.0); // +/- 5 degrees sway
    
    // Smoothly apply micro-adjustments
    safeSetServoAngle(NECK_ONE.channel, targetPan, NECK_ONE.minAngle, NECK_ONE.maxAngle);
    safeSetServoAngle(NECK_Y.channel, targetTilt, NECK_Y.minAngle, NECK_Y.maxAngle);
}
