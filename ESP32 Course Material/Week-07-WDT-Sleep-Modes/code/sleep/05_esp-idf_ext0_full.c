#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "esp_sleep.h"
#include "esp_log.h"

#define BUTTON_GPIO GPIO_NUM_33

static const char *TAG = "EXT0_WAKE";

RTC_DATA_ATTR static int boot_count = 0;

void app_main(void)
{
    boot_count++;

    ESP_LOGI(TAG, "Boot count: %d", boot_count);

    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

    if (cause == ESP_SLEEP_WAKEUP_EXT0) {
        ESP_LOGI(TAG, "Wakeup caused by EXT0 button");
    } else {
        ESP_LOGI(TAG, "Wakeup reason: %d", cause);
    }

    // If the pin was used as RTC IO during wake-up, deinitialize it
    // before using it again as normal GPIO.
    rtc_gpio_deinit(BUTTON_GPIO);

    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << BUTTON_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    ESP_ERROR_CHECK(gpio_config(&io_conf));

    ESP_ERROR_CHECK(esp_sleep_enable_ext0_wakeup(BUTTON_GPIO, 0));

    ESP_LOGI(TAG, "Going to deep sleep. Press button to wake.");

    vTaskDelay(pdMS_TO_TICKS(1000));

    esp_deep_sleep_start();
}
