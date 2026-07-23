# Week 7 — Watchdog Timer and Sleep Modes

## Topics
- Why reliability matters; what a Watchdog Timer is and how it improves robustness
- Typical failure scenarios; reset and recovery; WDT behavior on ESP32
- Power management as a design consideration; why sleep modes matter
- Idle, light sleep, and deep sleep; tradeoffs between performance, responsiveness, and power
- Sleep mode concepts in the ESP32

## Code

### `code/sleep/`
| File | Mode | Wake source |
|---|---|---|
| `01_arduino_deep_sleep_timer.ino` | Deep sleep | timer (5 s) |
| `02_esp-idf_deep_sleep_timer.c` | Deep sleep | timer (5 s) |
| `03_arduino_ext0_button.ino` | Deep sleep | EXT0 button (GPIO33) |
| `04_esp-idf_ext0_simple.c` | Deep sleep | EXT0 button (simple) |
| `05_esp-idf_ext0_full.c` | Deep sleep | EXT0 button (full, with `rtc_gpio_deinit`) |
| `06_arduino_ext1_several_buttons.ino` | Deep sleep | EXT1 (GPIO32 + GPIO33) |
| `07_esp-idf_ext1_several_buttons.c` | Deep sleep | EXT1 (GPIO32 + GPIO33) |
| `08_arduino_light_sleep.ino` | Light sleep | timer |
| `09_esp-idf_light_sleep.c` | Light sleep | timer |

### `code/wdt/`
| File | API style |
|---|---|
| `01_classic_wdt_api.ino` | classic `esp_task_wdt_init(seconds, panic)` |
| `02_wdt_config_struct_api.ino` | new `esp_task_wdt_config_t` + `esp_task_wdt_reconfigure()` |

## Notes
- [`notes/Sleep-and-WDT-Notes.md`](notes/Sleep-and-WDT-Notes.md) — what each demo shows + the page's download list
