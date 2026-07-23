#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "esp_sleep.h"

#define BUTTON_GPIO GPIO_NUM_33

RTC_DATA_ATTR int boot_count = 0;

void app_main(void)
{
    boot_count++;

    printf("Boot count: %d\n", boot_count);

    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0) {
        printf("Woke up because GPIO33 button was pressed\n");
    } else {
        printf("First boot or reset\n");
    }

    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_pullup_en(BUTTON_GPIO);
    gpio_pulldown_dis(BUTTON_GPIO);

    esp_sleep_enable_ext0_wakeup(BUTTON_GPIO, 0);

    printf("Sleeping now. Press button on GPIO33 to wake.\n");

    vTaskDelay(pdMS_TO_TICKS(1000));

    esp_deep_sleep_start();
}
