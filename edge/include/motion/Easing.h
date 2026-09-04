#pragma once

/**
 * @file Easing.h
 * @brief Header for Easing.h.
 */

#include <Arduino.h>

enum EasingType {
    EASE_IN_OUT_SINE,   // Medium-speed conversational articulation
    EASE_IN_OUT_CUBIC,  // Macro-movement posture shifts
    EASE_OUT_EXPO,      // Rapid micro-movements (blinks/saccades)
    NONE
};

/**
 * @brief Smooth acceleration and deceleration (Ease In-Out Sine)
 */
double easeInOutSine(double t);

/**
 * @brief Organic biological macro-movement acceleration (Ease In-Out Cubic)
 */
double easeInOutCubic(double t);

/**
 * @brief Rapid acceleration, slow deceleration (Ease Out Expo) for eye saccades
 */
double easeOutExpo(double t);

/**
 * @brief Simple pseudo-random Perlin-like noise generator
 * Returns a value between -1.0 and 1.0 based on time
 */
double generateSimpleNoise(uint32_t timeMs, float frequency);
