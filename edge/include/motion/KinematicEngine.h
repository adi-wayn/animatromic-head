#pragma once

/**
 * @file KinematicEngine.h
 * @brief Header for KinematicEngine.h.
 */

#include <Arduino.h>

#include "core/Config.h"
#include "motion/Easing.h"

struct ServoState {
    double startAngle;
    double targetAngle;
    uint32_t startTimeMs;
    uint32_t durationMs;
    EasingType easingType;
    bool isMoving;
};

class KinematicEngine {
   public:
    static KinematicEngine& getInstance() {
        static KinematicEngine instance;
        return instance;
    }
    KinematicEngine(const KinematicEngine&) = delete;
    void operator=(const KinematicEngine&) = delete;

    void begin();
    void updateKinematics();
    void triggerMove(const ServoConfig& config, double targetAngle, int durationMs,
                     EasingType easingType);

   private:
    KinematicEngine();
    ServoState states[16];
    double currentAngles[16];

    /**
     * @brief We will no longer handle noise for idle inside updateKinematics as requested
     * Task 2.3 moves idle behaviors (saccades, blinks) to SystemTasks/PoseController
     */
};
