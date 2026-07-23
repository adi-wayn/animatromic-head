# ESP32 Logging with `esp_log.h` — Student Summary

*Structured logs, log levels, tags, and how to use them correctly in ESP-IDF.*
*(Source attachment: `esp32_logging_summary.pdf`, 12 KiB)*

## Main idea
A log is not just printed text. In ESP-IDF, a log message carries a **level** such as error or warning, a **tag** that identifies the module, and usually a **timestamp**. This makes debugging much easier than using random print statements.

## 1. What is a log?
With Arduino, many beginners use `Serial.print()` to see values and messages. That works, but it is just raw text output.

In ESP-IDF, `esp_log.h` gives us a proper logging system. A log answers three questions at once: **what happened**, **where it happened**, and **how serious it is**.

## 2. Why not only use `Serial.print()`?
Because `Serial.print()` has no built-in idea of **error**, **warning**, or **normal information**. It also does not automatically organize output by module. In larger projects, the terminal becomes a wall of text. Logging is more structured and easier to filter.

## 3. Basic syntax
The normal pattern is:

```c
#include "esp_log.h"
static const char *TAG = "GPIO";
ESP_LOGI(TAG, "LED initialized");
ESP_LOGW(TAG, "Button looks noisy");
ESP_LOGE(TAG, "Sensor init failed");
```

The **TAG** is a short name for the source file or module, such as `"MAIN"`, `"GPIO"`, `"UART"`, or `"WIFI"`. A good tag helps us understand which part of the system produced the message.

## 4. What does the output usually look like?
```
I (1250) GPIO: LED initialized
W (2310) GPIO: Button looks noisy
E (4500) GPIO: Sensor init failed
```

A typical line contains the **log level**, a **timestamp**, the **tag**, and the message text.

## 5. The five main log levels
The summary lists five standard ESP-IDF log levels (macro / name / meaning / typical use), beginning with the macros shown above:

| Macro | Name |
|---|---|
| `ESP_LOGE` | Error |
| `ESP_LOGW` | Warning |
| `ESP_LOGI` | Info |
| `ESP_LOGD` | Debug |
| `ESP_LOGV` | Verbose |

> *Note: the per-level "meaning / typical use" table was cut off in the page export; the five
> macros above are the standard ESP-IDF set referenced by the summary. See the source PDF for the
> full table.*
