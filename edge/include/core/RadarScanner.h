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
    
    // Call in task loop
    void update();

    // Pause / Resume the radar sweep (to prevent electrical noise during audio playback)
    void setPaused(bool paused);

private:
    RadarScanner();
    
    // State machine for sweeping
    bool movingForward;
    double currentAngle;
    
    // Sweep constants
    const double MIN_ANGLE = 70.0; // Prevent looking too far right (avoids detecting the head itself)
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
    bool isPaused;

    // Helper functions
    float getDistanceCm();
    double calculateHeadPanAngle(float distanceCm, double radarAngle);
};

#endif // RADAR_SCANNER_H
