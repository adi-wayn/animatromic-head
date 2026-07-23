# Button Debouncing — Student Handout

*A student handout for ESP32 / Arduino digital input experiments.*
*(Source attachment: `Debounce_Student_Handout.pdf`, 106.6 KiB)*

**Core idea:** a mechanical push button does not change cleanly from 0 to 1 or from 1 to 0. For a few milliseconds the contacts bounce, and the microcontroller may see several false transitions. Debouncing is the process of filtering these false transitions so one real press becomes one logical event.

## 1. Why do we need debouncing?
A push button is a mechanical device. When you press it, the metal contacts do not settle instantly. They hit, rebound, touch again, and may oscillate for a short time. Electrically, that means one press may look like many very fast presses.

In a digital system, the ESP32 samples the pin repeatedly. If the signal bounces, the software may interpret one physical action as several separate events. This becomes especially problematic when a button is used to toggle an LED, increase a counter, move through a menu, or trigger a one-time action.

## 2. Ideal input versus real mechanical input
In an ideal world, a button signal would change once and then remain stable. Real buttons usually behave differently for a few milliseconds.

| Situation | Ideal signal | Real bouncing signal |
|---|---|---|
| Press | `1 1 1 1 0 0 0 0 0` | `1 1 1 0 1 0 0 1 0 0 0` |
| Release | `0 0 0 0 1 1 1 1 1` | `0 0 0 1 0 1 1 0 1 1 1` |

## 3. What can go wrong without debouncing?
Suppose the button should toggle an LED once per press. Without debouncing, the software may see several falling edges during one physical press. The LED may toggle two, three, or five times. To the user, the circuit appears random or unreliable.

**Bad idea:** toggle immediately whenever the pin reads LOW.

```c
if (digitalRead(BUTTON_PIN) == LOW) {
    led_state = !led_state;
}
```

> See the working software-debounce implementations in
> `../code/gpio_input/02_arduino_debounce.ino` and `../code/gpio_input/04_esp-idf_debounce.cpp`.
