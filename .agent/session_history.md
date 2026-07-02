# Session History: 2026-06-13

This log acts as persistent context memory for the next coding assistant session.

## 1. Summary of Changes Today
* **Staggered Boot Centering:** Servos are centered one-by-one with 500ms–1000ms delays at startup (configured in `main.cpp`) to prevent high startup current surges over USB.
* **Resting Idle Detach:** Implemented `detachServo(channel)` in `driver.cpp` and `driver.h`. The jaw sweep task now detaches the PWM signal to the jaw servo immediately when stationary during hold delays. This stops high-pitched hums/whining and prevents overheating.
* **Jaw Workaround Angles:** Shifted center to `110.0°` (safety range: `85.0°` to `135.0°`) to avoid extreme lockup zones at the far left and right.

## 2. Hardware / Diagnostic Findings
* The jaw servo (SG90, Channel 5) makes a high-pitched "spinning/moving fast" sound but does not physically move the jaw.
* The serial port `/dev/cu.usbserial-0001` occasionally busy-locks due to orphaned background scripts (can be freed via `kill <PID>` of the locking python process).

## 3. Next Steps for Tomorrow's Session
1. **Physical Inspection:**
   * Detach the bottom jaw part to check if the glue joint holding the servo horn to the plastic spline has broken or slipped.
   * Power down the system and gently push the jaw left and right. If it swings completely free without clicky resistance, the internal gears are stripped and the SG90 needs to be replaced.
2. **Firmware Verification:**
   * Once the physical alignment/connection is fixed, plug in the USB and the 5V power adapter.
   * Observe if the new slow sweep (`[85.0°, 135.0°]` with `110.0°` center) successfully rotates both left and right.
