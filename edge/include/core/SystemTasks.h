#pragma once

/**
 * @file SystemTasks.h
 * @brief Header for SystemTasks.h.
 */

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

/**
 * @brief ─────────────────────────────────────────────────
 * Hardware Timer ISR Semaphore (60 Hz kinematics)
 * Given by onKinematicsTimer() ISR, taken by kinematicsTask.
 * Declared here so ISR and task can share the handle.
 * @brief ─────────────────────────────────────────────────
 */
extern SemaphoreHandle_t kinematicsTriggerSem;

/**
 * @brief ─────────────────────────────────────────────────
 * FreeRTOS Task Entry Points
 * @brief ─────────────────────────────────────────────────
 */
void kinematicsTask(void* pvParameters);
void staggeredBootTask(void* pvParameters);
void networkTask(void* pvParameters);
void idleBehaviorTask(void* pvParameters);
void audioUplinkTask(void* pvParameters);
void audioDownlinkTask(void* pvParameters);
void telemetryTask(void* pvParameters);
void powerWatchdogTask(void* pvParameters);
void radarScannerTask(void* pvParameters);
