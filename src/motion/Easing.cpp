#include "motion/Easing.h"
#include <math.h>

double easeInOutSine(double t) {
    if (t < 0.0) t = 0.0;
    if (t > 1.0) t = 1.0;
    return -(cos(PI * t) - 1.0) / 2.0;
}

double easeOutExpo(double t) {
    if (t < 0.0) t = 0.0;
    if (t >= 1.0) return 1.0;
    return 1.0 - pow(2.0, -10.0 * t);
}

// A simple wave superposition to mimic Perlin noise without a heavy library
double generateSimpleNoise(uint32_t timeMs, float frequency) {
    float t = (float)timeMs * frequency / 1000.0f;
    // Combine multiple sine waves at different frequencies and phases
    double wave1 = sin(t);
    double wave2 = sin(t * 1.5f + 1.2f) * 0.5f;
    double wave3 = sin(t * 0.7f + 2.4f) * 0.75f;
    
    double result = (wave1 + wave2 + wave3) / 2.25f; // Normalize roughly to -1.0 .. 1.0
    
    if (result > 1.0) result = 1.0;
    if (result < -1.0) result = -1.0;
    
    return result;
}
