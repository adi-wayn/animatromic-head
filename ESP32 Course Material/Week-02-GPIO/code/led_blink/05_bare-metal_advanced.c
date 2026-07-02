#include <stdint.h>

void app_main(void)
{
    volatile uint32_t *IO_MUX_GPIO2      = (volatile uint32_t *)0x3FF49040;
    volatile uint32_t *GPIO_ENABLE_W1TS  = (volatile uint32_t *)0x3FF44024;
    volatile uint32_t *GPIO_OUT_W1TS     = (volatile uint32_t *)0x3FF44008;
    volatile uint32_t *GPIO_OUT_W1TC     = (volatile uint32_t *)0x3FF4400C;

    // Select GPIO function for pin 2
    *IO_MUX_GPIO2 &= ~(0x7 << 12);
    *IO_MUX_GPIO2 |=  (2 << 12);

    // Enable GPIO2 as output
    *GPIO_ENABLE_W1TS = (1 << 2);

    while (1) {
        *GPIO_OUT_W1TS = (1 << 2);

        for (volatile int i = 0; i < 800000; i++) { }

        *GPIO_OUT_W1TC = (1 << 2);

        for (volatile int i = 0; i < 800000; i++) { }
    }
}
