# Week 2 — Microcontroller 101 and GPIO in the ESP32

## Topics
- What a microcontroller is and how it differs from a general-purpose processor; CPU, memory, peripherals, buses
- The ESP32 as a modern microcontroller platform; intro to its documentation and datasheets
- The role of Arduino as an accessible software layer
- GPIO fundamentals: output vs input pins, digital logic levels, signal interpretation
- Push buttons and switches as digital inputs; mechanical bouncing and debouncing (software + hardware)
- Pull-up and pull-down resistors; internal vs external pull resistors in the ESP32

## Code

### `code/led_blink/` — blink an LED, three abstraction levels
| File | Layer | Notes |
|---|---|---|
| `01_arduino.ino` | Arduino | LED on GPIO14 + built-in LED GPIO2 |
| `02_esp-idf_gpio14.cpp` | ESP-IDF | `gpio_reset_pin` + `gpio_set_direction`, `vTaskDelay` |
| `03_esp-idf_gpio18_config.c` | ESP-IDF | uses `gpio_config_t`; **contains a deliberate/leftover bug** (references undeclared `in_cfg`) |
| `04_bare-metal_naive.c` | Bare metal | direct register pointers; explains `W1TS` vs writing the whole `OUT_REG` |
| `05_bare-metal_advanced.c` | Bare metal | adds IO_MUX function selection |
| `06_bare-metal_full.c` | Bare metal | full register map with macros, drive strength, pull config |
| `diagram.json` | WokWi | simulator wiring (ESP32 DevKit-C v4 + red LED on pin 14) |

### `code/gpio_input/` — read a button, drive LEDs
| File | Layer | Notes |
|---|---|---|
| `01_arduino.ino` | Arduino | button GPIO4 (`INPUT_PULLUP`) → LEDs |
| `02_arduino_debounce.ino` | Arduino | millis()-based software debounce, toggles on press |
| `03_esp-idf.cpp` | ESP-IDF | `gpio_get_level` polling |
| `04_esp-idf_debounce.cpp` | ESP-IDF | tick-based debounce |
| `05_bare-metal.c` | Bare metal | reads `GPIO_IN_REG`, drives GPIO14 + GPIO2 |

## Notes (conceptual summaries reproduced from the page)
- [`notes/Delays-in-ESP32-Summary.md`](notes/Delays-in-ESP32-Summary.md) — task delay vs busy wait, ticks, `vTaskDelay` vs `vTaskDelayUntil`
- [`notes/Logs-in-ESP32-Summary.md`](notes/Logs-in-ESP32-Summary.md) — `esp_log.h`, log levels, tags
- [`notes/Debounce-Summary.md`](notes/Debounce-Summary.md) — why buttons bounce and how to filter it

## Exercises (download from course page)
- `GPIO_Lesson_ESP32.pdf` (113 KiB) — exercise
- `GPIO_Lesson_Solutions_ESP32.pdf` (99 KiB) — solutions
