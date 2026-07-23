#include <stdint.h>
void app_main(void)
{
    volatile uint32_t *IO_MUX_GPIO14 = (volatile uint32_t *)0x3FF49030;
    volatile uint32_t *IO_MUX_GPIO2  = (volatile uint32_t *)0x3FF49040;
    volatile uint32_t *IO_MUX_GPIO4  = (volatile uint32_t *)0x3FF49044;
    volatile uint32_t *GPIO_ENABLE_W1TS = (volatile uint32_t *)0x3FF44024;
    volatile uint32_t *GPIO_OUT_W1TS    = (volatile uint32_t *)0x3FF44008;
    volatile uint32_t *GPIO_OUT_W1TC    = (volatile uint32_t *)0x3FF4400C;
    volatile uint32_t *GPIO_IN_REG      = (volatile uint32_t *)0x3FF4403C;
    // Select GPIO function for pin 14
    *IO_MUX_GPIO14 &= ~(0x7 << 12);
    *IO_MUX_GPIO14 |= (2 << 12);
    // Select GPIO function for pin 2
    *IO_MUX_GPIO2 &= ~(0x7 << 12);
    *IO_MUX_GPIO2 |= (2 << 12);
    // Select GPIO function for pin 4
    *IO_MUX_GPIO4 &= ~(0x7 << 12);
    *IO_MUX_GPIO4 |= (2 << 12);
    // Enable pull-up on GPIO4
    *IO_MUX_GPIO4 |= (1 << 8);
    // For pull up and puul-down -> *IO_MUX_GPIO4 |= (1 << 7);
    // Enable GPIO14 and GPIO2 as outputs
    *GPIO_ENABLE_W1TS = (1 << 14) | (1 << 2);
    while (1) {
        int button_state = (*GPIO_IN_REG >> 4) & 1;
        if (button_state == 0) {
            // Button pressed -> turn LEDs on
            *GPIO_OUT_W1TS = (1 << 14) | (1 << 2);
        } else {
            // Button not pressed -> turn LEDs off
            *GPIO_OUT_W1TC = (1 << 14) | (1 << 2);
        }
    }
}
/* one can also write the if code like this:
if (button_state == 0) {
    *GPIO_OUT_W1TS = (1 << 14);
    *GPIO_OUT_W1TS = (1 << 2);
} else {
    *GPIO_OUT_W1TC = (1 << 14);
    *GPIO_OUT_W1TC = (1 << 2);
}
*/
