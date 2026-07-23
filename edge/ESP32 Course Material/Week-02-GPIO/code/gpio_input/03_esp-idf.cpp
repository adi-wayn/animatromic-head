#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#define LED_PIN GPIO_NUM_14
#define LED_BUILT_IN_PIN GPIO_NUM_2
#define BUTTON_PIN GPIO_NUM_4
extern "C" void app_main(void)
{
    // Reset pins to a known state
    gpio_reset_pin(LED_PIN);
    gpio_reset_pin(LED_BUILT_IN_PIN);
    gpio_reset_pin(BUTTON_PIN);
    // Configure LED pins as outputs
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_BUILT_IN_PIN, GPIO_MODE_OUTPUT);
    // Configure button pin as input with internal pull-up
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLUP_ONLY);
    while (1) {
        int button_state = gpio_get_level(BUTTON_PIN);
        if (button_state == 0) {
            gpio_set_level(LED_PIN, 1);
            gpio_set_level(LED_BUILT_IN_PIN, 1);
        } else {
            gpio_set_level(LED_PIN, 0);
            gpio_set_level(LED_BUILT_IN_PIN, 0);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
