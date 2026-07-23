#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#define BUTTON_GPIO GPIO_NUM_18

volatile int32_t counter = 0;
volatile bool pressed = false;

static void IRAM_ATTR button_isr_handler(void *arg)
{
    counter++;
    pressed = true;
}

void app_main(void)
{
    /*gpio_config_t button_config = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };

    gpio_config(&button_config);
    */

    gpio_reset_pin(BUTTON_GPIO);

    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);

    gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_ONLY);

    gpio_set_intr_type(BUTTON_GPIO, GPIO_INTR_NEGEDGE);

    gpio_install_isr_service(0);

    gpio_isr_handler_add(BUTTON_GPIO, button_isr_handler, NULL);

    printf("Press the button on GPIO18.\n");

    while (1) {
        if (pressed) {
            printf("Button pressed %ld times.\n", counter);
            pressed = false;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
