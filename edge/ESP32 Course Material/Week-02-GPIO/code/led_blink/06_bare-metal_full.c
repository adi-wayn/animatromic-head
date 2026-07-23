#include <stdint.h>

// =========================
// Bare-metal register access
// Target: classic ESP32
// =========================

// Helper macros
#define REG32(addr) (*(volatile uint32_t *)(addr))
#define BIT(n)      (1U << (n))

// Base addresses
#define DR_REG_GPIO_BASE    0x3FF44000U
#define DR_REG_IO_MUX_BASE  0x3FF49000U

// GPIO registers
#define GPIO_OUT_REG            REG32(DR_REG_GPIO_BASE + 0x04)
#define GPIO_OUT_W1TS_REG       REG32(DR_REG_GPIO_BASE + 0x08)
#define GPIO_OUT_W1TC_REG       REG32(DR_REG_GPIO_BASE + 0x0C)

#define GPIO_ENABLE_REG         REG32(DR_REG_GPIO_BASE + 0x20)
#define GPIO_ENABLE_W1TS_REG    REG32(DR_REG_GPIO_BASE + 0x24)
#define GPIO_ENABLE_W1TC_REG    REG32(DR_REG_GPIO_BASE + 0x28)

// IO_MUX register for GPIO2
#define IO_MUX_GPIO2_REG        REG32(DR_REG_IO_MUX_BASE + 0x40)

// IO_MUX fields for classic ESP32
#define MCU_SEL_S               12
#define MCU_SEL_M               (0x7U << MCU_SEL_S)
#define FUN_DRV_S               10
#define FUN_DRV_M               (0x3U << FUN_DRV_S)
#define FUN_IE                  BIT(9)
#define FUN_WPU                 BIT(8)
#define FUN_WPD                 BIT(7)

// On classic ESP32, value 2 selects normal GPIO function in IO_MUX
#define PIN_FUNC_GPIO           2U

#define LED_GPIO                2U

static void delay_cycles(volatile uint32_t cycles)
{
    while (cycles--) {
        __asm__ volatile ("nop");
    }
}

static void gpio2_init_as_output(void)
{
    // Route pad to GPIO function:
    // clear function select bits [14:12]
    IO_MUX_GPIO2_REG &= ~MCU_SEL_M;

    // set function select = 2 (GPIO)
    IO_MUX_GPIO2_REG |= (PIN_FUNC_GPIO << MCU_SEL_S);

    // optional: medium drive strength
    IO_MUX_GPIO2_REG &= ~FUN_DRV_M;
    IO_MUX_GPIO2_REG |= (2U << FUN_DRV_S);

    // no pull-up / pull-down for simple LED output
    IO_MUX_GPIO2_REG &= ~FUN_WPU;
    IO_MUX_GPIO2_REG &= ~FUN_WPD;

    // enable output driver for GPIO2
    GPIO_ENABLE_W1TS_REG = BIT(LED_GPIO);
}

static void gpio2_set_high(void)
{
    GPIO_OUT_W1TS_REG = BIT(LED_GPIO);
}

static void gpio2_set_low(void)
{
    GPIO_OUT_W1TC_REG = BIT(LED_GPIO);
}

void app_main(void)
{
    gpio2_init_as_output();

    while (1) {
        gpio2_set_high();
        delay_cycles(800000);

        gpio2_set_low();
        delay_cycles(800000);
    }
}
//
// Created by Roi Yozevitch, Phd on 19/03/2026.
//
