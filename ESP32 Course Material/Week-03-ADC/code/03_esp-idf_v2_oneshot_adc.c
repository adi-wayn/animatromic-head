#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

#define LED_GPIO GPIO_NUM_21
#define ADC_CHANNEL ADC_CHANNEL_3 // GPIO4 on ESP32-S3 ADC1

int mapLdrToDelay(int adcValue)
{
    if (adcValue < 0) adcValue = 0;
    if (adcValue > 4095) adcValue = 4095;

    return 100 + ((4095 - adcValue) * 900) / 4095; // 100..1000 ms
}

void app_main(void)
{
    // LED setup
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    // ADC setup
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&init_config, &adc1_handle);

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12,
    };
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL, &config);

    while (1) {
        int adcValue;
        adc_oneshot_read(adc1_handle, ADC_CHANNEL, &adcValue);

        int delayMs = mapLdrToDelay(adcValue);

        printf("ADC = %d, Delay = %d ms\n", adcValue, delayMs);

        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(delayMs));

        gpio_set_level(LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(delayMs));
    }
}
