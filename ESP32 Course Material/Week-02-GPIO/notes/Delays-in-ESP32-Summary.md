# ESP32 Delays — Student Summary

*FreeRTOS delays, ticks, and the difference between task sleep and busy waiting.*
*(Source attachment: `esp32_delay_summary.pdf`, 9.9 KiB)*

## Main idea
On the ESP32, not every "delay" means the same thing. A task delay gives CPU time back to the system. A busy loop keeps the CPU occupied.

## 1. Three ways to "wait"
- **FreeRTOS task delay** — the current task goes to sleep for some time, and other tasks can run.
- **Arduino `delay()`** — a higher-level delay interface. On ESP32 it lives in an RTOS-based environment, but for beginners it is still easiest to think of it as "wait in milliseconds".
- **Bare-metal busy wait** — the CPU spins in a loop and does almost no useful work during that time.

The most important distinction is this: **sleeping a task** is not the same as **burning CPU cycles**.

## 2. What is a task?
In ESP-IDF, FreeRTOS manages **tasks**. A task is a flow of code that the scheduler can pause, resume, and switch between. If one task is waiting, another task can use the CPU.

Example idea: one task blinks an LED, another receives UART data, and another handles communication.

## 3. What does `vTaskDelay()` do?
`vTaskDelay()` delays the current task for a given number of OS ticks.

```c
vTaskDelay(pdMS_TO_TICKS(500));
```

Meaning: convert 500 milliseconds into ticks, then block this task for that many ticks. During this time, the task is not running.

Use it when you simply want to wait a bit between actions and exact periodic timing is not critical.

## 4. What does `vTaskDelayUntil()` do?
`vTaskDelayUntil()` is for periodic tasks that should run at a steady rate.

```c
TickType_t xLastWakeTime = xTaskGetTickCount();
const TickType_t xPeriod = pdMS_TO_TICKS(100);
while (1) {
    read_sensor();
    process_data();
    vTaskDelayUntil(&xLastWakeTime, xPeriod);
}
```

This does not mean "sleep 100 ms from now". It means "wake me at the next scheduled 100 ms point".

### When to choose each function
| Function | Meaning | Good for |
|---|---|---|
| `vTaskDelay()` | Relative delay from the moment the function is called | Simple pauses, LED blinking, slowing a loop, retrying later |
| `vTaskDelayUntil()` | Periodic wake-up based on a stable schedule | Sampling, control loops, regular status updates, fixed-rate tasks |

## 5. What is an OS tick?
An **OS tick** is the regular heartbeat of the RTOS. The tick interrupt lets the scheduler keep time and manage delayed tasks.

If the tick rate is 1000 Hz, then one OS tick happens every 1 ms.

```
tick period = 1 / configTICK_RATE_HZ
```

## 6. What is `configTICK_RATE_HZ`?
`configTICK_RATE_HZ` is the number of OS ticks per second.
- If `configTICK_RATE_HZ = 1000`, then the RTOS tick period is 1 ms.
- If `configTICK_RATE_HZ = 100`, then the RTOS tick period is 10 ms.

So the same tick count can represent very different real times on different systems.

## 7. Why convert milliseconds to ticks?
FreeRTOS delay functions work in **ticks**, but humans usually think in **milliseconds**. That is why we use `pdMS_TO_TICKS(...)`.

```
ticks = (milliseconds × configTICK_RATE_HZ) / 1000
```

Example:
- If `configTICK_RATE_HZ = 1000`, then `pdMS_TO_TICKS(500) = 500` ticks.
- If `configTICK_RATE_HZ = 100`, then `pdMS_TO_TICKS(500) = 50` ticks.

In both cases, the real delay is about 500 ms.
