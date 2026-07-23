#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_sleep.h"
#include "esp_log.h"

#define uS_TO_S_FACTOR 1000000ULL
#define SLEEP_TIME_SEC 5

static const char *TAG = "LIGHT_SLEEP_DEMO";

void app_main(void)
{
    int counter = 0;

    ESP_LOGI(TAG, "Light sleep demo");

    while (1) {
        counter++;

        ESP_LOGI(TAG, "Before light sleep. Counter = %d", counter);

        // Enable timer wake-up after 5 seconds
        ESP_ERROR_CHECK(
            esp_sleep_enable_timer_wakeup(SLEEP_TIME_SEC * uS_TO_S_FACTOR)
        );

        // Enter light sleep
        esp_light_sleep_start();

        // Execution continues here after waking up
        ESP_LOGI(TAG, "Woke up from light sleep");

        esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

        if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
            ESP_LOGI(TAG, "Wakeup caused by timer");
        } else {
            ESP_LOGI(TAG, "Wakeup reason: %d", wakeup_reason);
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
