#pragma once

/**
 * @file PoseController.h
 * @brief Header for PoseController.h.
 */

#include <stdint.h>

#include "motion/Easing.h"

class PoseController {
   public:
    static PoseController& getInstance() {
        static PoseController instance;
        return instance;
    }
    PoseController(const PoseController&) = delete;
    void operator=(const PoseController&) = delete;

    void executePose(const char* intent);

    /**
     * @brief Organic micro-movements (saccades)
     */
    void generateOrganicSaccade(uint32_t timeMs);

    /**
     * @brief Real-time lip sync driven by audio amplitude (0.0 to 1.0)
     */
    void syncJawToAmplitude(float intensity);

   private:
    PoseController() {}

    /**
     * @brief Primitives
     */
    void lookLeft(int durationMs = 500, EasingType easing = EASE_IN_OUT_SINE);
    void lookRight(int durationMs = 500, EasingType easing = EASE_IN_OUT_SINE);
    void lookUp(int durationMs = 500, EasingType easing = EASE_IN_OUT_SINE);
    void lookDown(int durationMs = 500, EasingType easing = EASE_IN_OUT_SINE);
    void tiltRight(int durationMs = 500, EasingType easing = EASE_IN_OUT_SINE);
    void tiltLeft(int durationMs = 500, EasingType easing = EASE_IN_OUT_SINE);

    void blinkRight(int durationMs = 150, EasingType easing = EASE_OUT_EXPO);
    void blinkLeft(int durationMs = 150, EasingType easing = EASE_OUT_EXPO);
    void blink(int durationMs = 150, EasingType easing = EASE_OUT_EXPO);

    void jawOpen(int durationMs = 300, EasingType easing = EASE_IN_OUT_SINE);
    void jawClose(int durationMs = 300, EasingType easing = EASE_IN_OUT_SINE);
    void jawLeft(int durationMs = 300, EasingType easing = EASE_IN_OUT_SINE);
    void jawRight(int durationMs = 300, EasingType easing = EASE_IN_OUT_SINE);

    /**
     * @brief Macros
     */
    void expressHappy();
    void expressSad();
    void expressThinking();
    void expressAngry();
    void expressSurprised();
    void resetToNeutral();

    // --- Strict Atomic Building Blocks (Base Primitives) ---
    /**
     * @brief Only these functions are permitted to communicate with KinematicEngine.
     */
    void moveNeckPan(double angle, int durationMs, EasingType easing);
    void moveNeckTilt(double angle, int durationMs, EasingType easing);
    void moveNeckRoll(double angle, int durationMs, EasingType easing);
    void moveEyesPan(double angle, int durationMs, EasingType easing);
    void moveEyesTilt(double angle, int durationMs, EasingType easing);
    void moveEyelidLeft(double angle, int durationMs, EasingType easing);
    void moveEyelidRight(double angle, int durationMs, EasingType easing);
    void moveJawUD(double angle, int durationMs, EasingType easing);
    void moveJawLR(double targetAngle, int durationMs, EasingType easing);  // Clamps internally
};
