#include "core/RadarScanner.h"
#include "core/Config.h"
#include "core/PowerManager.h"
#include "motion/KinematicEngine.h"
#include "hardware/PCA9685_Driver.h"
#include "controllers/AnimatronicHead.h"
#include <math.h>
#include "controllers/AudioManager.h"
#include "core/VoiceClips.h"


RadarScanner::RadarScanner() 
    : movingForward(true), currentAngle(MIN_ANGLE), 
      state(RadarState::SWEEPING), stateStartTime(0), tentativeAngle(90.0), lastLogTime(0), isPaused(false) {
}

void RadarScanner::begin() {
    pinMode(RADAR_TRIG_PIN, OUTPUT);
    pinMode(RADAR_ECHO_PIN, INPUT);
    digitalWrite(RADAR_TRIG_PIN, LOW);
}


void RadarScanner::setPaused(bool paused) {
    isPaused = paused;
}

float RadarScanner::getDistanceCm() {
    digitalWrite(RADAR_TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(RADAR_TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(RADAR_TRIG_PIN, LOW);

    long duration = pulseIn(RADAR_ECHO_PIN, HIGH, 15000);
    if (duration > 100) {
        return duration * 0.034 / 2.0;
    }
    return 0.0;
}

double RadarScanner::calculateHeadPanAngle(float distanceCm, double radarAngle) {
    // Math logic: Radar 90 is forward. <90 is Right. >90 is Left.
    // If Radar sees something at 80 (Right), the head (NECK_ONE) should ALSO turn Right (<90).
    // We will use a slightly exaggerated direct mapping to make it feel natural,
    // since the geometric offset was causing it to look left when the radar pointed right.
    // We'll map the radar's angle directly with an offset to compensate for the physical 45-deg physical mount.
    
    // Instead of complex Cartesian trig (which caused the "head looks left when radar points right" feeling),
    // let's do a direct angular mapping:
    // Radar 90 (center) -> Head 90 (center)
    // Radar 80 (right)  -> Head 80 (right)
    // Radar 135 (left)  -> Head 135 (left)
    return radarAngle;
}

void RadarScanner::update() {
    if (isPaused) return;

    safeSetServoAngle(RADAR_SERVO_CHANNEL, currentAngle, 0.0, 180.0);
    vTaskDelay(pdMS_TO_TICKS(15)); 

    float distanceCm = getDistanceCm();
    uint32_t now = millis();

    // Print a log every 2 seconds to give visibility!
    if (now - lastLogTime >= 2000) {
        const char* stateStr = (state == RadarState::SWEEPING) ? "SWEEPING" :
                               (state == RadarState::VERIFYING_TARGET) ? "VERIFYING" : "COOLDOWN";
        Serial.printf("[Radar] State: %s | Angle: %.1f° | Dist: %.1f cm\n", stateStr, currentAngle, distanceCm);
        lastLogTime = now;
    }

    switch(state) {
        case RadarState::SWEEPING:
            if (distanceCm > 0.0 && distanceCm <= RADAR_THRESHOLD_CM) {
                state = RadarState::VERIFYING_TARGET;
                stateStartTime = now;
                tentativeAngle = calculateHeadPanAngle(distanceCm, currentAngle);
                Serial.printf("[Radar] Target spotted at %.1f cm (Radar %.1f°). Pausing to verify...\n", distanceCm, currentAngle);
            } else {
                if (movingForward) {
                    currentAngle += SWEEP_STEP;
                    if (currentAngle >= MAX_ANGLE) { 
                        currentAngle = MAX_ANGLE; 
                        movingForward = false; 
                        vTaskDelay(pdMS_TO_TICKS(200)); 
                    }
                } else {
                    currentAngle -= SWEEP_STEP;
                    if (currentAngle <= MIN_ANGLE) { 
                        currentAngle = MIN_ANGLE; 
                        movingForward = true; 
                        vTaskDelay(pdMS_TO_TICKS(200)); 
                    }
                }
            }
            break;

        case RadarState::VERIFYING_TARGET:
            if (distanceCm > 0.0 && distanceCm <= RADAR_THRESHOLD_CM) {
                if (now - stateStartTime >= 2000) {
                    Serial.printf("[Radar] Target verified! Sending NECK_ONE to %.1f°\n", tentativeAngle);
                    
                    if (PowerManager::getInstance().isLowPower()) {
                        Serial.println("[Radar] Waking up Animatronic Head!");
                        PowerManager::getInstance().enterFullPower();
                    }
                    AnimatronicHead::getInstance().updateActivityTimestamp();

                    KinematicEngine::getInstance().triggerMove(
                        NECK_ONE, 
                        tentativeAngle, 
                        1000, 
                        EasingType::EASE_IN_OUT_CUBIC
                    );

                    // Pick a random phrase and play it!
                    SystemState oldState = AnimatronicHead::getInstance().getState();
                    AnimatronicHead::getInstance().setState(SystemState::SPEAKING_SYNCING);
                    
                    int clipIndex = esp_random() % 3;
                    if (clipIndex == 0) {
                        AudioManager::getInstance().playLocalClip(voice1_raw, voice1_raw_len);
                    } else if (clipIndex == 1) {
                        AudioManager::getInstance().playLocalClip(voice2_raw, voice2_raw_len);
                    } else {
                        AudioManager::getInstance().playLocalClip(voice3_raw, voice3_raw_len);
                    }
                    
                    AnimatronicHead::getInstance().setState(oldState);

                    state = RadarState::COOLDOWN;
                    stateStartTime = millis(); // Reset now because playLocalClip takes ~1 second
                }
            } else {
                Serial.println("[Radar] Target lost during verification. Resuming sweep.");
                state = RadarState::SWEEPING;
            }
            break;

        case RadarState::COOLDOWN:
            if (distanceCm > 0.0 && distanceCm <= RADAR_THRESHOLD_CM) {
                AnimatronicHead::getInstance().updateActivityTimestamp();
                stateStartTime = now; 
            }
            
            if (now - stateStartTime >= 3000) {
                Serial.println("[Radar] Target left. Cooldown finished. Resuming sweep.");
                state = RadarState::SWEEPING;
            }
            break;
    }
}
