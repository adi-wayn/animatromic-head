#ifndef RADAR_SCANNER_H
#define RADAR_SCANNER_H

#include <Arduino.h>

class RadarScanner {
public:
    static RadarScanner& getInstance() {
        static RadarScanner instance;
        return instance;
    }
    RadarScanner(const RadarScanner&) = delete;
    void operator=(const RadarScanner&) = delete;

    void begin();
    
    // Designed to be called continuously from the FreeRTOS radarScannerTask
    void update();

private:
    RadarScanner();
    
    // State machine for sweeping
    bool movingForward;
    double currentAngle;
    
    // Sweep constants
    const double MIN_ANGLE = 80.0; // Prevent looking too far right (avoids detecting the head itself)
    const double MAX_ANGLE = 180.0;
    const double SWEEP_STEP = 2.0;

    enum class RadarState {
        SWEEPING,
        VERIFYING_TARGET,
        COOLDOWN
    };

    RadarState state;
    uint32_t stateStartTime;
    double tentativeAngle;
    uint32_t lastLogTime;

    // Helper functions
    float getDistanceCm();
    double calculateHeadPanAngle(float distanceCm, double radarAngle);
};

#endif // RADAR_SCANNER_H
