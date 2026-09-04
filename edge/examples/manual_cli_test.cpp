/**
 * @file manual_cli_test.cpp
 * @brief Implementation of manual_cli_test.cpp.
 */
#include <Arduino.h>

#include "controllers/AnimatronicHead.h"

AnimatronicHead head;

void cliTask(void* pvParameters) {
    (void)pvParameters;

    Serial.println("CLI Test Ready. Press:");
    Serial.println("'i': Initialize PCA9685 Driver (DO THIS FIRST)");
    Serial.println("'1': Look Left");
    Serial.println("'2': Look Right");
    Serial.println("'3': Look Up");
    Serial.println("'4': Look Down");
    Serial.println("'5': Tilt Right");
    Serial.println("'6': Tilt Left");
    Serial.println("'r': Blink Right Eyelid");
    Serial.println("'l': Blink Left Eyelid");
    Serial.println("'b': Blink Both Eyelids");
    Serial.println("'o': Jaw Open");
    Serial.println("'c': Jaw Close");
    Serial.println("'x': Jaw Left");
    Serial.println("'y': Jaw Right");
    Serial.println("'h': Express Happy");
    Serial.println("'s': Express Sad");
    Serial.println("'t': Express Thinking");
    Serial.println("'m': Toggle Idle Mode (Micro-movements)");
    Serial.println("'0': Reset to Neutral");
    Serial.println("-------------------------");

    while (true) {
        if (Serial.available()) {
            char c = Serial.read();
            switch (c) {
                case 'i':
                    Serial.println("Initializing PCA9685 Driver...");
                    head.begin();
                    Serial.println("Driver initialized! Now you can move servos.");
                    break;
                case '1':
                    Serial.println("Looking Left...");
                    head.lookLeft();
                    break;
                case '2':
                    Serial.println("Looking Right...");
                    head.lookRight();
                    break;
                case '3':
                    Serial.println("Looking Up...");
                    head.lookUp();
                    break;
                case '4':
                    Serial.println("Looking Down...");
                    head.lookDown();
                    break;
                case '5':
                    Serial.println("Tilting Right...");
                    head.tiltRight();
                    break;
                case '6':
                    Serial.println("Tilting Left...");
                    head.tiltLeft();
                    break;
                case 'r':
                    Serial.println("Blinking Right...");
                    head.blinkRight();
                    break;
                case 'l':
                    Serial.println("Blinking Left...");
                    head.blinkLeft();
                    break;
                case 'b':
                    Serial.println("Blinking Both...");
                    head.blink();
                    break;
                case 'o':
                    Serial.println("Jaw Opening...");
                    head.jawOpen();
                    break;
                case 'c':
                    Serial.println("Jaw Closing...");
                    head.jawClose();
                    break;
                case 'x':
                    Serial.println("Jaw Left...");
                    head.jawLeft();
                    break;
                case 'y':
                    Serial.println("Jaw Right...");
                    head.jawRight();
                    break;
                case 'h':
                    Serial.println("Expressing Happy...");
                    head.expressHappy();
                    break;
                case 's':
                    Serial.println("Expressing Sad...");
                    head.expressSad();
                    break;
                case 't':
                    Serial.println("Expressing Thinking...");
                    head.expressThinking();
                    break;
                case 'm':
                    Serial.println("Toggling Idle Mode...");
                    head.toggleIdleMode();
                    break;
                case '0':
                    Serial.println("Resetting to Neutral...");
                    head.resetToNeutral();
                    break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }

    Serial.println("\n\n-----------------------------------");
    Serial.println("SYSTEM BOOTED SUCCESSFULLY! (115200 Baud)");
    Serial.println("PCA9685 IS CURRENTLY OFF (Not Initialized).");
    Serial.println("-----------------------------------\n");

    xTaskCreatePinnedToCore(cliTask, "CLITask", 4096, NULL, 1, NULL, 1);
}

void loop() {
    // Continuously update idle micro-movements if enabled
    head.updateIdleMicroMovements();
    vTaskDelay(pdMS_TO_TICKS(20));  // ~50Hz update rate
}
