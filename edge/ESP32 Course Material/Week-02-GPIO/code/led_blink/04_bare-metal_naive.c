#include <stdint.h>

#define GPIO2 2

volatile uint32_t *GPIO_ENABLE_W1TS = (volatile uint32_t *)0x3FF44024;
volatile uint32_t *GPIO_OUT_W1TS    = (volatile uint32_t *)0x3FF44008;
volatile uint32_t *GPIO_OUT_W1TC    = (volatile uint32_t *)0x3FF4400C;

void delay_cycles(volatile uint32_t count)
{
    while (count--) { }
}

void app_main(void)
{
    *GPIO_ENABLE_W1TS = (1 << GPIO2);

    while (1) {
        *GPIO_OUT_W1TS = (1 << GPIO2);
        delay_cycles(800000);

        *GPIO_OUT_W1TC = (1 << GPIO2);
        delay_cycles(800000);
    }
}

/*
Very important distinction

*GPIO_OUT_W1TS = (1 << GPIO2);

is not the same as:

*GPIO_OUT_REG = (1 << GPIO2);

If you write to GPIO_OUT_REG, you replace the whole register value.

That means:
1. bit 2 becomes 1
2. all other bits become whatever your written value says, usually 0

So that could turn off other LEDs. If you write to GPIO_OUT_W1TS, you only set selected bits.
Everything else stays as it was.
*/
