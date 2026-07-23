#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#define BUTTON_GPIO GPIO_NUM_18

static QueueHandle_t gpio_evt_queue = NULL;

static void IRAM_ATTR button_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;

    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

void app_main(void)
{
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));

    gpio_config_t button_config = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };

    gpio_config(&button_config);

    gpio_install_isr_service(0);

    gpio_isr_handler_add(BUTTON_GPIO,
                         button_isr_handler,
                         (void *)BUTTON_GPIO);

    printf("Press the button on GPIO18.\n");

    uint32_t io_num;
    int counter = 0;

    while (1) {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            counter++;

            printf("Interrupt from GPIO %lu. Count = %d\n",
                   io_num,
                   counter);
        }
    }
}
