# Week 7 — Sleep Modes & Watchdog Timer (notes)

The course page provides infographics, PowerPoint lessons, and student-summary PDFs for this week
(see download list below). The conceptual material lives mostly in those attachments; the runnable
demonstrations are reproduced under `../code/`.

## Sleep modes — what the code demonstrates
- **Deep sleep + timer wake** — `RTC_DATA_ATTR` keeps `bootCount` across resets; `esp_sleep_enable_timer_wakeup` + `esp_deep_sleep_start`. After deep sleep the chip restarts from the top (`setup`/`app_main`), so code after `esp_deep_sleep_start()` never runs.
- **EXT0 wake (single button)** — `esp_sleep_enable_ext0_wakeup(pin, level)` wakes on one specific RTC GPIO (GPIO33).
- **EXT1 wake (several buttons)** — `esp_sleep_enable_ext1_wakeup(bitmask, ESP_EXT1_WAKEUP_ANY_HIGH)` wakes on any of several RTC GPIOs (GPIO32/GPIO33); `esp_sleep_get_ext1_wakeup_status()` tells you which pin fired.
- **Light sleep** — `esp_light_sleep_start()` returns and execution *continues from where it stopped* (unlike deep sleep), so a counter in `loop()` keeps incrementing.

## Watchdog Timer — what the code demonstrates
A Task Watchdog (TWDT) resets the system if a subscribed task stops "feeding" it.
- Subscribe with `esp_task_wdt_add(NULL)`, feed with `esp_task_wdt_reset()`.
- Both demos run normally for 10 s, then enter an infinite loop that stops feeding the WDT → the watchdog eventually fires and resets/panics the chip.
- `01_classic_wdt_api.ino` uses the older `esp_task_wdt_init(timeout_seconds, panic)` signature.
- `02_wdt_config_struct_api.ino` uses the newer `esp_task_wdt_config_t` struct + `esp_task_wdt_reconfigure()`.

## Material on the page (download)
- Sleep Mode InfoGraphics
- `ESP32_Sleep_Modes_Instructor_Lesson.pptx` (509.3 KiB)
- `ESP32_Sleep_Modes_Student_Summary.pdf` (221.3 KiB)
- WDT InfoGraphics
- `ESP32_WDT_Student_Summary.pdf` (36.1 KiB)
- `ESP32_WDT_Lesson_Presentation.pptx` (814.3 KiB)
