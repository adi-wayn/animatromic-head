#ifndef EASING_H
#define EASING_H

#include <Arduino.h>

// Smooth acceleration and deceleration (Ease In-Out Sine)
double easeInOutSine(double t);

// Rapid acceleration, slow deceleration (Ease Out Expo) for eye saccades
double easeOutExpo(double t);

// Simple pseudo-random Perlin-like noise generator
// Returns a value between -1.0 and 1.0 based on time
double generateSimpleNoise(uint32_t timeMs, float frequency);

#endif // EASING_H
