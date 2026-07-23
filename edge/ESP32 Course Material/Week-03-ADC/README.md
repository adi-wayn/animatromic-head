# Week 3 — ADC (Analog-to-Digital Conversion)

## Topics
- Analog vs digital signals; why real-world signals are often analog
- The role of the ADC in embedded systems
- Sampling (discrete time) and quantization (discrete amplitude)
- Resolution and number of bits; voltage range and reference considerations
- Quantization error and measurement limits; noise and practical inaccuracy
- ADC concepts on the ESP32; reading sensor values through ADC channels

## Code — LDR → blink-rate demo (read light, change blink speed)
| File | Layer | API |
|---|---|---|
| `01_arduino_ldr_blink.ino` | Arduino | `analogRead`, `analogReadResolution(12)`, `analogSetPinAttenuation` |
| `02_esp-idf_v1_legacy_adc.c` | ESP-IDF | legacy `driver/adc.h` (`adc1_get_raw`) |
| `03_esp-idf_v2_oneshot_adc.c` | ESP-IDF | new `esp_adc/adc_oneshot.h` API |

> All three target the **ESP32-S3** (LDR on GPIO4 / ADC1, LED on GPIO21) and map a 12-bit reading (0–4095) to a 100–1000 ms blink delay: brighter → faster.

## Notes
- [`notes/ADC-Summary.md`](notes/ADC-Summary.md) — ADC fundamentals handout (analog vs digital, sampling, SAR, quantization, resolution…)

## Other material on the page
- SAR ADC Video Explanation (embedded video)
