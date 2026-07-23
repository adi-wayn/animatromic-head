#ifndef EASING_H
#define EASING_H

#include <Arduino.h>

enum EasingType {
    EASE_IN_OUT_SINE, // Medium-speed conversational articulation
    EASE_IN_OUT_CUBIC, // Macro-movement posture shifts
    EASE_OUT_EXPO, // Rapid micro-movements (blinks/saccades)
    NONE
};

// Smooth acceleration and deceleration (Ease In-Out Sine)
double easeInOutSine(double t);

// Organic biological macro-movement acceleration (Ease In-Out Cubic)
double easeInOutCubic(double t);

// Rapid acceleration, slow deceleration (Ease Out Expo) for eye saccades
double easeOutExpo(double t);

// Simple pseudo-random Perlin-like noise generator
// Returns a value between -1.0 and 1.0 based on time
double generateSimpleNoise(uint32_t timeMs, float frequency);

#endif // EASING_H
