#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_sleep.h"
#include "esp_log.h"

#define BUTTON_1 GPIO_NUM_32
#define BUTTON_2 GPIO_NUM_33

#define BUTTON_PIN_BITMASK ((1ULL << BUTTON_1) | (1ULL << BUTTON_2))

static const char *TAG = "EXT1_WAKE";

RTC_DATA_ATTR static int boot_count = 0;

void app_main(void)
{
    boot_count++;

    ESP_LOGI(TAG, "Boot count: %d", boot_count);

    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

    if (cause == ESP_SLEEP_WAKEUP_EXT1) {
        uint64_t wakeup_mask = esp_sleep_get_ext1_wakeup_status();

        ESP_LOGI(TAG, "Wakeup caused by EXT1");

        if (wakeup_mask & (1ULL << BUTTON_1)) {
            ESP_LOGI(TAG, "GPIO32 caused wakeup");
        }

        if (wakeup_mask & (1ULL << BUTTON_2)) {
            ESP_LOGI(TAG, "GPIO33 caused wakeup");
        }
    } else {
        ESP_LOGI(TAG, "Wakeup reason: %d", cause);
    }

    gpio_config_t io_conf = {
        .pin_bit_mask = BUTTON_PIN_BITMASK,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    ESP_ERROR_CHECK(gpio_config(&io_conf));

    ESP_ERROR_CHECK(
        esp_sleep_enable_ext1_wakeup(
            BUTTON_PIN_BITMASK,
            ESP_EXT1_WAKEUP_ANY_HIGH
        )
    );

    ESP_LOGI(TAG, "Going to deep sleep. Press GPIO32 or GPIO33 button.");

    vTaskDelay(pdMS_TO_TICKS(1000));

    esp_deep_sleep_start();
}
