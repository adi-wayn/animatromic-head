#include "AnimatronicHead.h"
#include "driver.h"
#include "neck_one.h"
#include "neck_y.h"
#include "eyes_x.h"
#include "eyes_y.h"
#include "jaw_ud.h"
#include "eyelid_left.h"
#include "eyelid_right.h"
#include "neck_roll.h"

AnimatronicHead::AnimatronicHead() {
    // Initialize trackers to center/default positions
    currentNeckOne = NECK_ONE_CENTER_ANGLE;
    currentNeckY = NECK_Y_CENTER_ANGLE;
    currentNeckRoll = NECK_ROLL_CENTER_ANGLE;
    currentEyesX = EYES_X_CENTER_ANGLE;
    currentEyesY = EYES_Y_CENTER_ANGLE;
    currentJaw = JAW_UD_CENTER_ANGLE;
    currentEyelidLeft = EYELID_LEFT_MIN_ANGLE;
    currentEyelidRight = EYELID_RIGHT_MIN_ANGLE;
}

void AnimatronicHead::begin() {
    initDriver();
    // Intentionally NOT sending any servo commands here. 
    // This ensures a "silent boot" to prevent power spikes or immediate crazy movements.
    // The servos will only move when you press a key in the CLI.
}

double& AnimatronicHead::getTrackedAngle(uint8_t channel) {
    if (channel == NECK_ONE_CHANNEL) return currentNeckOne;
    if (channel == NECK_Y_CHANNEL) return currentNeckY;
    if (channel == NECK_ROLL_CHANNEL) return currentNeckRoll;
    if (channel == EYES_X_CHANNEL) return currentEyesX;
    if (channel == EYES_Y_CHANNEL) return currentEyesY;
    if (channel == JAW_UD_CHANNEL) return currentJaw;
    if (channel == EYELID_LEFT_CHANNEL) return currentEyelidLeft;
    if (channel == EYELID_RIGHT_CHANNEL) return currentEyelidRight;
    return currentNeckOne; // fallback
}

void AnimatronicHead::smoothMove(uint8_t channel, double targetAngle, double minLimit, double maxLimit, int durationMs) {
    double& startAngle = getTrackedAngle(channel);
    int steps = 20; 
    int delayPerStep = durationMs / steps;
    
    for (int i = 1; i <= steps; i++) {
        double currentAngle = startAngle + ((targetAngle - startAngle) * i / steps);
        safeSetServoAngle(channel, currentAngle, minLimit, maxLimit);
        vTaskDelay(pdMS_TO_TICKS(delayPerStep));
    }
    startAngle = targetAngle; // Update tracker
}

// --- Basic Directions ---

void AnimatronicHead::lookLeft() {
    smoothMove(NECK_ONE_CHANNEL, NECK_ONE_MAX_ANGLE, NECK_ONE_MIN_ANGLE, NECK_ONE_MAX_ANGLE, 500);
    smoothMove(EYES_X_CHANNEL, EYES_X_MAX_ANGLE, EYES_X_MIN_ANGLE, EYES_X_MAX_ANGLE, 300);
}

void AnimatronicHead::lookRight() {
    smoothMove(NECK_ONE_CHANNEL, NECK_ONE_MIN_ANGLE, NECK_ONE_MIN_ANGLE, NECK_ONE_MAX_ANGLE, 500);
    smoothMove(EYES_X_CHANNEL, EYES_X_MIN_ANGLE, EYES_X_MIN_ANGLE, EYES_X_MAX_ANGLE, 300);
}

void AnimatronicHead::lookUp() {
    smoothMove(NECK_Y_CHANNEL, NECK_Y_MAX_ANGLE, NECK_Y_MIN_ANGLE, NECK_Y_MAX_ANGLE, 500);
    smoothMove(EYES_Y_CHANNEL, EYES_Y_MIN_ANGLE, EYES_Y_MIN_ANGLE, EYES_Y_MAX_ANGLE, 300); 
}

void AnimatronicHead::lookDown() {
    smoothMove(NECK_Y_CHANNEL, NECK_Y_MIN_ANGLE, NECK_Y_MIN_ANGLE, NECK_Y_MAX_ANGLE, 500);
    smoothMove(EYES_Y_CHANNEL, EYES_Y_MAX_ANGLE, EYES_Y_MIN_ANGLE, EYES_Y_MAX_ANGLE, 300);
}

void AnimatronicHead::tiltRight() {
    // Up (higher angle) is tilt right
    smoothMove(NECK_ROLL_CHANNEL, NECK_ROLL_MAX_ANGLE, NECK_ROLL_MIN_ANGLE, NECK_ROLL_MAX_ANGLE, 500);
}

void AnimatronicHead::tiltLeft() {
    // Down (lower angle) is tilt left
    smoothMove(NECK_ROLL_CHANNEL, NECK_ROLL_MIN_ANGLE, NECK_ROLL_MIN_ANGLE, NECK_ROLL_MAX_ANGLE, 500);
}

// --- Eyelids ---

void AnimatronicHead::blinkRight() {
    safeSetServoAngle(EYELID_RIGHT_CHANNEL, EYELID_RIGHT_MAX_ANGLE, EYELID_RIGHT_MIN_ANGLE, EYELID_RIGHT_MAX_ANGLE);
    vTaskDelay(pdMS_TO_TICKS(150));
    safeSetServoAngle(EYELID_RIGHT_CHANNEL, EYELID_RIGHT_MIN_ANGLE, EYELID_RIGHT_MIN_ANGLE, EYELID_RIGHT_MAX_ANGLE);
    currentEyelidRight = EYELID_RIGHT_MIN_ANGLE;
}

void AnimatronicHead::blinkLeft() {
    safeSetServoAngle(EYELID_LEFT_CHANNEL, EYELID_LEFT_MAX_ANGLE, EYELID_LEFT_MIN_ANGLE, EYELID_LEFT_MAX_ANGLE);
    vTaskDelay(pdMS_TO_TICKS(150));
    safeSetServoAngle(EYELID_LEFT_CHANNEL, EYELID_LEFT_MIN_ANGLE, EYELID_LEFT_MIN_ANGLE, EYELID_LEFT_MAX_ANGLE);
    currentEyelidLeft = EYELID_LEFT_MIN_ANGLE;
}

void AnimatronicHead::blink() {
    safeSetServoAngle(EYELID_RIGHT_CHANNEL, EYELID_RIGHT_MAX_ANGLE, EYELID_RIGHT_MIN_ANGLE, EYELID_RIGHT_MAX_ANGLE);
    safeSetServoAngle(EYELID_LEFT_CHANNEL, EYELID_LEFT_MAX_ANGLE, EYELID_LEFT_MIN_ANGLE, EYELID_LEFT_MAX_ANGLE);
    vTaskDelay(pdMS_TO_TICKS(150));
    safeSetServoAngle(EYELID_RIGHT_CHANNEL, EYELID_RIGHT_MIN_ANGLE, EYELID_RIGHT_MIN_ANGLE, EYELID_RIGHT_MAX_ANGLE);
    safeSetServoAngle(EYELID_LEFT_CHANNEL, EYELID_LEFT_MIN_ANGLE, EYELID_LEFT_MIN_ANGLE, EYELID_LEFT_MAX_ANGLE);
    currentEyelidRight = EYELID_RIGHT_MIN_ANGLE;
    currentEyelidLeft = EYELID_LEFT_MIN_ANGLE;
}

// --- Emotions / Reactions ---

void AnimatronicHead::expressHappy() {
    safeSetServoAngle(EYELID_LEFT_CHANNEL, EYELID_LEFT_MIN_ANGLE, EYELID_LEFT_MIN_ANGLE, EYELID_LEFT_MAX_ANGLE);
    safeSetServoAngle(EYELID_RIGHT_CHANNEL, EYELID_RIGHT_MIN_ANGLE, EYELID_RIGHT_MIN_ANGLE, EYELID_RIGHT_MAX_ANGLE);
    currentEyelidLeft = EYELID_LEFT_MIN_ANGLE;
    currentEyelidRight = EYELID_RIGHT_MIN_ANGLE;
    smoothMove(NECK_Y_CHANNEL, NECK_Y_MAX_ANGLE, NECK_Y_MIN_ANGLE, NECK_Y_MAX_ANGLE, 300);
}

void AnimatronicHead::expressSad() {
    double halfClosedLeft = (EYELID_LEFT_MIN_ANGLE + EYELID_LEFT_MAX_ANGLE) / 2;
    double halfClosedRight = (EYELID_RIGHT_MIN_ANGLE + EYELID_RIGHT_MAX_ANGLE) / 2;
    safeSetServoAngle(EYELID_LEFT_CHANNEL, halfClosedLeft, EYELID_LEFT_MIN_ANGLE, EYELID_LEFT_MAX_ANGLE);
    safeSetServoAngle(EYELID_RIGHT_CHANNEL, halfClosedRight, EYELID_RIGHT_MIN_ANGLE, EYELID_RIGHT_MAX_ANGLE);
    currentEyelidLeft = halfClosedLeft;
    currentEyelidRight = halfClosedRight;
    smoothMove(NECK_Y_CHANNEL, NECK_Y_MIN_ANGLE, NECK_Y_MIN_ANGLE, NECK_Y_MAX_ANGLE, 1000);
}

void AnimatronicHead::expressThinking() {
    smoothMove(NECK_ONE_CHANNEL, NECK_ONE_CENTER_ANGLE + 15, NECK_ONE_MIN_ANGLE, NECK_ONE_MAX_ANGLE, 600);
    smoothMove(NECK_Y_CHANNEL, NECK_Y_CENTER_ANGLE + 10, NECK_Y_MIN_ANGLE, NECK_Y_MAX_ANGLE, 600);
}

void AnimatronicHead::resetToNeutral() {
    smoothMove(NECK_ONE_CHANNEL, NECK_ONE_CENTER_ANGLE, NECK_ONE_MIN_ANGLE, NECK_ONE_MAX_ANGLE, 500);
    smoothMove(NECK_Y_CHANNEL, NECK_Y_CENTER_ANGLE, NECK_Y_MIN_ANGLE, NECK_Y_MAX_ANGLE, 500);
    smoothMove(NECK_ROLL_CHANNEL, NECK_ROLL_CENTER_ANGLE, NECK_ROLL_MIN_ANGLE, NECK_ROLL_MAX_ANGLE, 500);
    smoothMove(EYES_X_CHANNEL, EYES_X_CENTER_ANGLE, EYES_X_MIN_ANGLE, EYES_X_MAX_ANGLE, 500);
    smoothMove(EYES_Y_CHANNEL, EYES_Y_CENTER_ANGLE, EYES_Y_MIN_ANGLE, EYES_Y_MAX_ANGLE, 500);
    smoothMove(JAW_UD_CHANNEL, JAW_UD_CENTER_ANGLE, JAW_UD_MIN_ANGLE, JAW_UD_MAX_ANGLE, 500);
    smoothMove(EYELID_LEFT_CHANNEL, EYELID_LEFT_MIN_ANGLE, EYELID_LEFT_MIN_ANGLE, EYELID_LEFT_MAX_ANGLE, 300);
    smoothMove(EYELID_RIGHT_CHANNEL, EYELID_RIGHT_MIN_ANGLE, EYELID_RIGHT_MIN_ANGLE, EYELID_RIGHT_MAX_ANGLE, 300);
}
