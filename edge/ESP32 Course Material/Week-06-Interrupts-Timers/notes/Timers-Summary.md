# Timers — Notes & Summaries

## Why `volatile`? (from the interrupt examples)
The `counter` variable is marked as `volatile` because it is changed inside the ISR and read inside `loop()`. Without `volatile`, the compiler may assume that `counter` only changes inside normal code. But the ISR can change it "behind the compiler's back," like a tiny hardware goblin.

---

## Arduino timer vs ESP-IDF GPTimer — concept comparison

| Timer concept | Arduino code | ESP-IDF code | What changed |
|---|---|---|---|
| Timer object / handle | `hw_timer_t *My_timer = NULL;` | `gptimer_handle_t my_timer = NULL;` | Arduino uses `hw_timer_t`; ESP-IDF uses a GPTimer handle. Same idea: a variable that represents the hardware timer. |
| Timer ISR / callback function | `void IRAM_ATTR onTimer()` | `static bool IRAM_ATTR on_timer_alarm(...)` | Arduino ISR is very simple. ESP-IDF callback receives timer information and must return `true` or `false`. |
| Create / configure timer | `My_timer = timerBegin(0, 80, true);` | `gptimer_config_t timer_config = {...};` then `gptimer_new_timer(&timer_config, &my_timer);` | Arduino uses one function with timer number, prescaler, and direction. ESP-IDF first builds a configuration structure, then creates the timer. |
| Timer clock / resolution | Prescaler 80: 80 MHz / 80 = 1 MHz | `.resolution_hz = 1000000` | Arduino exposes the prescaler directly. ESP-IDF asks directly for the desired timer resolution. |
| Count direction | Third argument: `true` | `.direction = GPTIMER_COUNT_UP` | Same meaning: the timer counts upward. ESP-IDF writes it more explicitly. |
| Attach interrupt function | `timerAttachInterrupt(My_timer, &onTimer, true);` | `gptimer_event_callbacks_t callbacks = {...};` then `gptimer_register_event_callbacks(my_timer, &callbacks, NULL);` | Arduino directly attaches one ISR. ESP-IDF creates a callback structure and registers it. |
| Set alarm time | `timerAlarmWrite(My_timer, 1000000, true);` | `gptimer_alarm_config_t alarm_config = {...};` then `gptimer_set_alarm_action(my_timer, &alarm_config);` | Arduino sets the alarm in one function. ESP-IDF uses an alarm configuration structure. |
| Alarm value | `1000000` | `.alarm_count = 1000000` | Same meaning: 1,000,000 timer ticks. With 1 MHz timer resolution, that equals 1 second. |
| Auto-reload | Third argument: `true` | `.flags.auto_reload_on_alarm = true` | Same meaning: after the alarm fires, restart automatically and keep repeating. |
| Enable timer | `timerAlarmEnable(My_timer);` | `gptimer_enable(my_timer);` | Arduino enables the alarm. ESP-IDF explicitly enables the timer driver. |
| Start timer | Mostly implicit after alarm enable | `gptimer_start(my_timer);` | ESP-IDF separates "enable" and "start." |

---

## Timers PDF Summary (1) — intro
**ESP32 Timers — Student Summary** *(`ESP32_Timers_Student_Summary.pdf`, 197.7 KiB)*

> Core idea: a hardware timer counts clock ticks independently of the main program. When the timer reaches an alarm value, it can trigger an interrupt. This lets the ESP32 perform periodic actions without blocking code using `delay()`.
> Covers: Arduino IDE and ESP-IDF. Course: Microcontrollers with the ESP32.

## Timers PDF Summary (advanced) — intro
**Advanced GPTimer in ESP32 — Student Summary** *(`ESP32_Advanced_GPTimer_Student_Summary.pdf`, 186.2 KiB)*

> Continues the basic timer lesson. Explains advanced GPTimer features in ESP-IDF: raw count reading, one-shot alarms, dynamic alarm updates, callback context, power management, cache safety, ISR-safe APIs, and resource cleanup. The goal is practical understanding.
> Primary reference: Espressif Systems, ESP-IDF Programming Guide, General Purpose Timer (GPTimer), ESP32, stable documentation. Accessed May 2026.
