#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#define LED_PIN GPIO_NUM_14
#define LED_BUILT_IN_PIN GPIO_NUM_2
#define BUTTON_PIN GPIO_NUM_4
static bool led_state = false;
static int stable_button_state = 1;
static int last_reading = 1;
static TickType_t last_debounce_time = 0;
static const TickType_t debounce_delay = pdMS_TO_TICKS(20);
extern "C" void app_main(void)
{
    gpio_reset_pin(LED_PIN);
    gpio_reset_pin(LED_BUILT_IN_PIN);
    gpio_reset_pin(BUTTON_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_BUILT_IN_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_PIN, GPIO_PULLUP_ONLY);
    while (1) {
        int current_reading = gpio_get_level(BUTTON_PIN);
        TickType_t now = xTaskGetTickCount();
        if (current_reading != last_reading) {
            last_debounce_time = now;
        }
        if ((now - last_debounce_time) > debounce_delay) {
            if (current_reading != stable_button_state) {
                stable_button_state = current_reading;
                if (stable_button_state == 0) {
                    led_state = !led_state;
                }
            }
        }
        last_reading = current_reading;
        gpio_set_level(LED_PIN, led_state);
        gpio_set_level(LED_BUILT_IN_PIN, led_state);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
