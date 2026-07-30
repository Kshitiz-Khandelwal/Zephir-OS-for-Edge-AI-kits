# 06 — Thread Priorities & Preemption

This program directly solves the RTOS exercise: *"Design a system in which an emergency LED must always blink immediately when a button is pressed, even while other LEDs continue their normal operation."*

## CONNECTION

| Component | Pin | Raspberry Pi Pico |
|-----------|-----|-------------------|
| **LED 1 (+)** | - | GP14 |
| **LED 2 (+)** | - | GP15 |
| **LED Emergency (+)** | - | GP13 |
| **Button / Sensor** | OUT | GP16 |
| **Button VCC** | VCC | 3.3V |
| **All Grounds (-)** | GND | GND |

> **Note:** Use 330Ω resistors in series with each LED to prevent them from burning out!

## 🧠 Concepts Explained: Thread Priorities & Preemption

In Zephyr RTOS, every thread is assigned a **Priority**. 
* **Lower numbers = Higher priority.** (e.g. Priority 2 is higher than Priority 7).

### The Scenario
1. **Thread 1 (Priority 7):** Blinks LED 1 every 500ms.
2. **Thread 2 (Priority 7):** Blinks LED 2 every 1000ms.
3. **Emergency Thread (Priority 2):** Monitors the button on GP16.

### How Preemption Works
If the Emergency Thread was the same priority (7) as the others, the RTOS would "time-slice" between them. The button press might have a slight delay before being processed if the OS was busy handling the other LEDs.

By giving the Emergency Thread a higher priority (Priority 2), we enable **Preemption**.
- When the button is pressed, the Emergency Thread immediately says "I have work to do!"
- The RTOS instantly **preempts** (pauses/interrupts) Threads 1 and 2, no matter what they were doing.
- The Emergency LED flashes immediately.
- Once the Emergency Thread goes back to `k_sleep()` (waiting for the next button press), the OS resumes Threads 1 and 2 exactly where they left off.

This guarantees that critical emergency actions happen with absolute minimum latency, without destroying the normal operation of the rest of the system!

## Pre-compiled Firmware
You can flash this program directly to your board without building it yourself.
1. Hold **BOOTSEL** on your Pico 2.
2. Plug it into your PC.
3. Drag and drop [`06_thread_priorities.uf2`](06_thread_priorities.uf2) into the `RPI-RP2` drive.
