#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "esp_log.h"

#define TIME_TO_SLEEP_SEC 5
#define uS_TO_S_FACTOR 1000000ULL

static const char *TAG = "SLEEP_DEMO";

RTC_DATA_ATTR static int boot_count = 0;

static void print_wakeup_reason(void)
{
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

    switch (cause) {
        case ESP_SLEEP_WAKEUP_TIMER:
            ESP_LOGI(TAG, "Wakeup caused by timer");
            break;

        case ESP_SLEEP_WAKEUP_EXT0:
            ESP_LOGI(TAG, "Wakeup caused by EXT0");
            break;

        case ESP_SLEEP_WAKEUP_EXT1:
            ESP_LOGI(TAG, "Wakeup caused by EXT1");
            break;

        case ESP_SLEEP_WAKEUP_TOUCHPAD:
            ESP_LOGI(TAG, "Wakeup caused by touchpad");
            break;

        default:
            ESP_LOGI(TAG, "Wakeup was not caused by deep sleep: %d", cause);
            break;
    }
}

void app_main(void)
{
    boot_count++;

    ESP_LOGI(TAG, "ESP32 woke up");
    ESP_LOGI(TAG, "Boot count: %d", boot_count);

    print_wakeup_reason();

    ESP_ERROR_CHECK(
        esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_SEC * uS_TO_S_FACTOR)
    );

    ESP_LOGI(TAG, "Going to deep sleep for %d seconds", TIME_TO_SLEEP_SEC);

    vTaskDelay(pdMS_TO_TICKS(1000));

    esp_deep_sleep_start();
}
