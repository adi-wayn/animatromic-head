# Mechanical Assembly & 3D Model Notes

This document captures assembly details and parts reference for the 3D-printed Animatronic Skull.

## 1. 3D Print Reference
* **Model:** Animatronic Skull by Djfx on Thingiverse
* **Source URL:** https://www.thingiverse.com/thing:2456550
* **Printed Parts Inventory (located in `docs/Animatronic Skull - 2456550/files/`):**
  * `FullSkull.stl`: Main combined skull print.
  * `jaw.stl`: The jaw mechanism.
  * `teeth.stl`: Teeth parts.
  * `eyeballs.stl` & `eyemech.stl`: Eye mounting and linkage mechanism.
  * `Neck.stl` & `base.stl`: Neck movement brackets and baseplate.
  * `LeftSkull.stl` & `RightSkull.stl` & `skullparts.stl`: Skull exterior panels.

## 2. Mechanical Design Features
* **Jaw Articulation:** The jaw is designed to swing slightly forward when opening to simulate realistic human speech patterns. It has two degrees of freedom:
  1. Open/Close pitch.
  2. Left/Right yaw.
* **Top Head Alignment:** Uses a wired iron structure driven by a **HX5010** standard servo at the top/rear skull to stabilize and maintain horizontal alignment.
* **Eye Linkage:** Linked mechanism to control yaw/pitch coordinates using only 2 servos for the pair.
* **Neck:** Stewart-platform-like neck driven by 3 MG945 standard metal-gear servos using ball links (4.8x3x17mm) to support pitch, roll, and yaw.

## 3. Current Mechanical Assembly State
* **Active Servos:** Four SG90 micro-servos are currently active:
  * Eyes Y-Axis (CH0)
  * Eyes X-Axis (CH1)
  * Eyelids Open/Close (CH4)
  * Bottom Jaw Left/Right (CH5)
* **Teeth Interference:** The teeth (`teeth.stl`) on the bottom jaw clash with the upper teeth if rotated too far left or right. The mechanical clearance permits **at most ±25° of rotation** around the shifted center alignment.
* **Jaw Servo Physical Issue (2026-06-13):** During testing, the jaw servo was observed making a high-pitched whirring/spinning sound ("moving fast") without translating to physical jaw movement. This indicates either a stripped plastic gear train inside the SG90 micro-servo, a loose/broken glue joint between the servo horn and the spline, or a physical linkage blockage. A physical inspection/repair is scheduled for the next session.
