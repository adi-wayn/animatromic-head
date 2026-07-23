// General Purpose Timer (GPTimer) - ESP32 - ESP-IDF Programming Guide.
// NOTE (verbatim from course page): the trailing `while (1) { vTaskDelay(...) }` loop appears
// AFTER the closing brace of app_main on the page - kept as-is.
#include <stdio.h>
#include <stdbool.h>

#include "driver/gpio.h"
#include "driver/gptimer.h"

#define LED_GPIO GPIO_NUM_21

static bool led_state = false;
static gptimer_handle_t my_timer = NULL;

static bool IRAM_ATTR on_timer_alarm(gptimer_handle_t timer,
                                     const gptimer_alarm_event_data_t *edata,
                                     void *user_data)
{
    led_state = !led_state;
    gpio_set_level(LED_GPIO, led_state);

    return false;
}

void app_main(void)
{
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO, 0);

    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000, // 1 MHz = 1 tick per microsecond
    };

    gptimer_new_timer(&timer_config, &my_timer);

    gptimer_event_callbacks_t callbacks = {
        .on_alarm = on_timer_alarm,
    };

    gptimer_register_event_callbacks(my_timer, &callbacks, NULL);

    gptimer_alarm_config_t alarm_config = {
        .alarm_count = 2000000, // 2,000,000 ticks = 2 seconds
        .reload_count = 0,
        .flags.auto_reload_on_alarm = true,
    };

    gptimer_set_alarm_action(my_timer, &alarm_config);

    gptimer_enable(my_timer);

    gptimer_start(my_timer);

    printf("GPTimer interrupt started.\n");
}
while (1) {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
