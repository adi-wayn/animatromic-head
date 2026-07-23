#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"

#define LDR_CHANNEL ADC1_GPIO4_CHANNEL
#define LED_GPIO GPIO_NUM_21

int mapLdrToDelay(int adcValue)
{
    if (adcValue < 0) adcValue = 0;
    if (adcValue > 4095) adcValue = 4095;

    return 100 + ((4095 - adcValue) * 900) / 4095; // 100..1000 ms
}

void app_main(void)
{
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(LDR_CHANNEL, ADC_ATTEN_DB_12);

    while (1) {
        int adcValue = adc1_get_raw(LDR_CHANNEL);
        int delayMs = mapLdrToDelay(adcValue);

        printf("ADC = %d , Delay = %d ms\n", adcValue, delayMs);

        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(delayMs));

        gpio_set_level(LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(delayMs));
    }
}
