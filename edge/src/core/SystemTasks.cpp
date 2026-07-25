#include "core/SystemTasks.h"
#include <Arduino.h>
#include "controllers/AnimatronicHead.h"
#include "controllers/NetworkManager.h"
#include "controllers/AudioManager.h"
#include "hardware/PCA9685_Driver.h"
#include <math.h>

void kinematicsTask(void *pvParameters) {
  (void) pvParameters;
  while(true) {
    if (AnimatronicHead::getInstance().isBooted()) {
      // If we are currently speaking, drive the jaw from audio amplitude
      if (AnimatronicHead::getInstance().getState() == SystemState::SPEAKING_SYNCING) {
          float intensity = AudioManager::getInstance().getAmplitude();
          PoseController::getInstance().syncJawToAmplitude(intensity);
      }

      AnimatronicHead::getInstance().updateKinematics();
    }
    vTaskDelay(pdMS_TO_TICKS(15)); // ~60Hz update loop
  }
}

void staggeredBootTask(void *pvParameters) {
  (void) pvParameters;
  Serial.println("Starting Staggered Boot Sequence...");
  
  AnimatronicHead::getInstance().begin();

  // Array of all servos to initialize safely
  ServoConfig servos[] = {NECK_ONE, NECK_Y, NECK_ROLL, EYES_X, EYES_Y, JAW_UD, JAW_LR, EYELID_LEFT, EYELID_RIGHT};
  int numServos = sizeof(servos) / sizeof(ServoConfig);

  for (int i = 0; i < numServos; i++) {
    Serial.printf("Initializing Servo Channel %d...\n", servos[i].channel);
    // Move to center/safe angle
    double initAngle = servos[i].centerAngle;
    if (servos[i].channel == EYELID_LEFT.channel) initAngle = EYELID_LEFT.minAngle;
    if (servos[i].channel == EYELID_RIGHT.channel) initAngle = EYELID_RIGHT.minAngle;
    
    safeSetServoAngle(servos[i].channel, initAngle, servos[i].minAngle, servos[i].maxAngle);
    
    // Wait 500ms for current spike to subside before turning on the next servo
    vTaskDelay(pdMS_TO_TICKS(500));
  }

  Serial.println("Staggered Boot Complete. System Ready.");
  AnimatronicHead::getInstance().setBooted(true);
  
  // Task is done, delete itself
  vTaskDelete(NULL);
}

// --- Service 1: Network Transport (Core 0, High Priority) ---
void networkTask(void *pvParameters) {
  (void) pvParameters;
  NetworkManager& network = NetworkManager::getInstance();
  network.begin();
  while (true) {
    network.update();
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

// --- Service 3: Idle Behaviors (Core 1, Low Priority) ---
void idleBehaviorTask(void *pvParameters) {
  (void) pvParameters;
  while (true) {
    AnimatronicHead& head = AnimatronicHead::getInstance();
    
    if (head.isBooted() && head.getState() == SystemState::IDLE_LISTENING) {
        // Trigger organic eye movement via the Facade
        head.triggerSaccade(millis());
        
        // 15% chance to blink using the generic executePose API!
        if (random(0, 100) < 15) {
            head.executePose("BLINK");
        }
        
        // Wait biological delay before next dart
        vTaskDelay(pdMS_TO_TICKS(random(800, 2500)));
    } else {
        vTaskDelay(pdMS_TO_TICKS(500));
    }
  }
}

// --- Service 4: Audio Uplink — Mic to Host (Core 0, High Priority) ---
void audioUplinkTask(void *pvParameters) {
  (void) pvParameters;
  AudioManager& audio = AudioManager::getInstance();
  audio.beginMic();

  uint8_t pcmBuffer[AUDIO_CHUNK_SIZE_BYTES];

  while (true) {
    size_t bytesRead = audio.readMicChunk(pcmBuffer, AUDIO_CHUNK_SIZE_BYTES);
    if (bytesRead > 0) {
      audio.sendToHost(pcmBuffer, bytesRead);
    }
    // No vTaskDelay needed — i2s_read blocks on DMA, yielding naturally
  }
}

// --- Service 5: Audio Downlink — Host TTS to Speaker (Core 0, High Priority) ---
void audioDownlinkTask(void *pvParameters) {
  (void) pvParameters;
  AudioManager& audio = AudioManager::getInstance();
  audio.beginSpeaker();

  uint8_t pcmBuffer[AUDIO_CHUNK_SIZE_BYTES];
  const float RMS_SCALER = 32768.0f; // 16-bit PCM max value

  while (true) {
    int bytesReceived = audio.receiveFromHost(pcmBuffer, AUDIO_CHUNK_SIZE_BYTES);
    if (bytesReceived > 0) {
      // Calculate RMS for Lip-Sync
      int16_t* samples = (int16_t*)pcmBuffer;
      int numSamples = bytesReceived / 2;
      float sumSquares = 0.0f;
      
      for (int i = 0; i < numSamples; i++) {
          float sample = (float)samples[i];
          sumSquares += sample * sample;
      }
      
      float rms = 0.0f;
      if (numSamples > 0) {
          rms = sqrt(sumSquares / numSamples);
      }
      
      // Normalize and boost intensity (same math as Host)
      float intensity = (rms / RMS_SCALER) * 3.0f;
      audio.setAmplitude(intensity);

      audio.writeToSpeaker(pcmBuffer, bytesReceived);
      // i2s_write blocks until DMA accepts — natural pacing
    } else {
      // No packet available — yield briefly to avoid busy-spinning
      // Decay amplitude slowly if silence (prevents jaw snapping closed on dropped packets)
      float currentAmp = audio.getAmplitude();
      if (currentAmp > 0.01f) {
          audio.setAmplitude(currentAmp * 0.8f); 
      } else {
          audio.setAmplitude(0.0f);
      }
      vTaskDelay(pdMS_TO_TICKS(1));
    }
  }
}
