#ifndef SYSTEM_TASKS_H
#define SYSTEM_TASKS_H

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// ─────────────────────────────────────────────────
//  Hardware Timer ISR Semaphore (60 Hz kinematics)
//  Given by onKinematicsTimer() ISR, taken by kinematicsTask.
//  Declared here so ISR and task can share the handle.
// ─────────────────────────────────────────────────
extern SemaphoreHandle_t kinematicsTriggerSem;

// ─────────────────────────────────────────────────
//  FreeRTOS Task Entry Points
// ─────────────────────────────────────────────────
void kinematicsTask(void *pvParameters);
void staggeredBootTask(void *pvParameters);
void networkTask(void *pvParameters);
void idleBehaviorTask(void *pvParameters);
void audioUplinkTask(void *pvParameters);
void audioDownlinkTask(void *pvParameters);
void telemetryTask(void *pvParameters);
void powerWatchdogTask(void *pvParameters);  // NEW: monitors inactivity, triggers power transitions

#endif
