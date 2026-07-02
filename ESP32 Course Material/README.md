# MicroControllers Course (ESP32 for EE & CE) — 2026

> Undergraduate Microcontroller Systems (ESP32) for Electrical Engineering & Computer Engineering.
> Extracted and organized from the official course Notion page.

- **Platform:** ESP32
- **Language:** C/C++ (Arduino, ESP-IDF & Bare Metal)
- **Emphasis:** embedded systems design, peripherals, RTOS, connectivity, and measurement
- **Instructor:** Dr. Roi Yozevitch — roiyo@Ariel.ac.il
- **Course number:** 7630410 · 3rd year · 3.5 credits · 3 h lecture + 1 h recitation
- **Assessment:** 100% final exam
- **Type:** Theoretical course (no hardware lab/project required)

**Source:** https://roi.notion.site/MicroControllers-Course-ESP32-for-EE-CE-2026-327e0f376194801c9db4e823a019b259
**Course page (short link):** https://tinyurl.com/5ch6kh2y

---

## How this folder is organized

| File / Folder | What's inside |
|---|---|
| [`Syllabus.md`](Syllabus.md) | Full course description, objectives, prerequisites, and the topic outline for all 11 weeks |
| [`Course-Info-and-Links.md`](Course-Info-and-Links.md) | Staff contacts, Zoom meeting, lecture recordings (EE + CE groups), Arduino playlist, reference docs |
| [`Provided-Files-Index.md`](Provided-Files-Index.md) | Every downloadable file on the course page (slides, PDFs, exercises) with sizes — **must be downloaded manually** |
| `Week-XX-*/` | Per-week folder: a `README.md` (topics + materials), `code/` (runnable source files), and `notes/` (conceptual summaries) where the course provides them |

### Weekly index

| Week | Topic | Has code | Has notes |
|---|---|:--:|:--:|
| 01 | [Programming Recap & Course Introduction](Week-01-Programming-Recap/) | — | — |
| 02 | [Microcontroller 101 & GPIO](Week-02-GPIO/) | ✅ | ✅ |
| 03 | [ADC (Analog-to-Digital Conversion)](Week-03-ADC/) | ✅ | ✅ |
| 04 | [Serial Communication 1: UART](Week-04-UART/) | ✅ | — |
| 05 | [Serial Communication 2: I2C & SPI](Week-05-I2C-SPI/) | — | — |
| 06 | [Interrupts, Counters & Timers](Week-06-Interrupts-Timers/) | ✅ | ✅ |
| 07 | [Watchdog Timer & Sleep Modes](Week-07-WDT-Sleep-Modes/) | ✅ | ✅ |
| 08 | [PWM](Week-08-PWM/) | — | — |
| 09 | [Wi-Fi Connectivity & IoT](Week-09-WiFi-IoT/) | — | — |
| 10 | [FreeRTOS, Part 1](Week-10-FreeRTOS-1/) | — | — |
| 11 | [FreeRTOS, Part 2](Week-11-FreeRTOS-2/) | — | — |

> Weeks 5, 8, 9, 10, 11 had only topic outlines on the course page at extraction time (no posted code/notes yet).

---

## Reference manuals (already in the parent project folder)

- `esp32_technical_reference_manual_en.pdf` (~9.7 MiB)
- `esp32_datasheet_en.pdf` (~966 KiB)

See `Course-Info-and-Links.md` for the ESP32 pinout diagram and Arduino tutorial links.

---

*Extracted 2026-06-04. Code blocks are reproduced verbatim from the course page; some contain intentional teaching mistakes or were written for the ESP32-S3 — check pin numbers and API versions against your own board before compiling.*
