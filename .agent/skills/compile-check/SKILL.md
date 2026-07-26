---
name: compile-check
description: >-
  Compiles the ESP32 firmware using PlatformIO to verify that all C++ code 
  changes are syntactically valid and compile successfully without needing
  to flash the hardware.
---

# Compile-Check Skill

## Overview
This skill runs `pio run` in the `edge` directory to verify that the ESP32 firmware compiles successfully.

## Quick Start
Run the attached script to verify compilation:
```bash
./.agent/skills/compile-check/compile.sh
```
