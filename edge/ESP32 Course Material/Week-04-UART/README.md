# Week 4 — Serial Communication, Part 1: UART

## Topics
- Why microcontrollers need serial communication; parallel vs serial
- UART as asynchronous serial communication; TX and RX lines
- Start bit, stop bit, data framing; baud rate and synchronization
- Common UART use cases: debugging, logging, communicating with modules
- UART in the ESP32; conceptual reading of UART traffic; pros/cons vs other interfaces

## Code
| File | What it does |
|---|---|
| `01_arduino_hello.ino` | Minimal `Serial.begin(115200)` "hello" loop on ESP32-S3, blinks LED on GPIO35 every 10 prints |
| `processing_heltec_oled/processing_mouse.pde` | **Processing** (desktop) sketch — sends mouse X/Y as two serial bytes to the board |
| `processing_heltec_oled/esp32_heltec_oled.ino` | ESP32 (Heltec WiFi Kit) receives the two bytes and shows MouseX/MouseY on the SSD1306 OLED |
| `processing_heltec_oled/platformio.ini` | PlatformIO config for the Heltec WiFi Kit 32 V3 board |

> The Processing + Heltec pair demonstrates a full PC ⇄ microcontroller UART link.

## Material on the page (download)
- `Serial Communication.pdf` (1.8 MiB) — UART presentation
- UART Poster
- `NEO-6_DataSheet_(GPS.G6-HW-09005).pdf` (866.1 KiB) — u-blox NEO-6 GPS datasheet
