#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define LED_GPIO GPIO_NUM_14   // Common onboard LED pin on many ESP32 dev boards

extern "C" void app_main(void)
{
    // Configure the LED pin as output
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    while (1) {
        gpio_set_level(LED_GPIO, 1);   // LED ON
        vTaskDelay(pdMS_TO_TICKS(500));

        gpio_set_level(LED_GPIO, 0);   // LED OFF
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
//
// Created by Roi Yozevitch, Phd on 19/03/2026.
//
