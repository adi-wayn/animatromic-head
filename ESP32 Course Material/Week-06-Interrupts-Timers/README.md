# Week 6 — Interrupts, Counters, and Timers

## Topics
- Polling vs interrupts; why interrupts are central in embedded systems
- Interrupt service routine (ISR) concept; external vs internal interrupts
- Latency, priority, and responsiveness
- Timer fundamentals; counters and timer-based events
- Prescalers, overflow, and periodic timing; measuring time and generating timed behavior
- Conceptual use of interrupts and timers in the ESP32

## Code

### `code/interrupts/`
| File | What it shows |
|---|---|
| `01_polling_example.ino` | basic polling of a button |
| `02_polling_disadvantages.ino` | how slow work makes polling miss events |
| `03_arduino_isr_counter.ino` | `attachInterrupt` ISR increments a `volatile` counter |
| `04_arduino_isr_debounce.ino` | ISR + `millis()` debounce |
| `05_esp-idf_isr.c` | `gpio_install_isr_service` / `gpio_isr_handler_add` |
| `06_esp-idf_queue.c` | ISR posts events to a FreeRTOS queue (`xQueueSendFromISR`) |
| `simple_framework_pseudocode.txt` | blocking loop vs interrupt-driven (pseudocode) |

### `code/timers/`
| File | What it shows |
|---|---|
| `01_arduino_simple_timer.ino` | `timerBegin` / `timerAttachInterrupt` / `timerAlarm` (new Arduino-ESP32 API; note legacy calls in comments — and a verbatim `timer`/`My_timer` typo) |
| `02_arduino_two_timers.ino` | two independent hardware timers at different periods |
| `03_esp-idf_gptimer.c` | ESP-IDF GPTimer with alarm callback (note: trailing `while` loop is outside `app_main` as on the page) |

## Notes
- [`notes/Timers-Summary.md`](notes/Timers-Summary.md) — "Why volatile?", the full Arduino↔ESP-IDF timer concept comparison table, and both timer-summary PDF intros

## Material on the page (download)
- Timer Infographic
- `ESP32_Timers_Student_Summary.pdf` (197.7 KiB)
- `ESP32_Advanced_GPTimer_Student_Summary.pdf` (186.2 KiB)
