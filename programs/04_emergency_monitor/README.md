# 04 — Emergency Monitor (Non-Blocking Timers + Threads)

Demonstrates how to use `k_uptime_get_32()` to create non-blocking timers (similar to `time.ticks_ms()` in MicroPython) for the main thread, while a second thread monitors a sensor and interrupts normal operation with an emergency flash sequence if triggered.

## CONNECTION

| LED | Pico GPIO |
|:---|:---|
| **RED** | GP15 |
| **AMB** | GP14 |
| **GRN** | GP13 |

**Touch Sensor:**

| Touch Sensor Pin | Pico |
|:---|:---|
| **I/O / OUT** | GP16 |
| **VCC** | 5V |
| **GND** | GND |

> **Note:** Use 330Ω resistors in series with each LED to prevent them from burning out!

## 🧠 Concepts Explained

### The Problem: Blocking Delays
If a microcontroller executes a `delay(1000)` to wait for an LED to blink, the processor sits there doing absolutely nothing. During that 1000ms:
- A sensor cannot be read.
- A display cannot be updated.
- A Wi-Fi transmission is delayed.
This is known as a **blocking operation**. It prevents the program from performing other work until the current operation finishes, making the system unresponsive.

### The Solution: Non-Blocking Timing
Instead of asking the CPU to *"Wait for 1000 ms"*, we ask it *"Has 1000 ms elapsed?"*

This is exactly what Program 04 does in Thread 1 using `k_uptime_get_32()`. 
```c
currentTime = k_uptime_get_32();
if (currentTime - lastLEDTime >= 1000) {
    blinkLED();
    lastLEDTime = currentTime;
}
```
The processor never stops running and looping. It simply checks if the required time has passed. If it hasn't, it skips the `if` block and is instantly free to do other things (like checking the emergency touch sensor). Since the processor loops millions of times a second, neither task blocks the other!

## Pre-compiled Firmware
You can flash this program directly to your board without building it yourself.
1. Hold **BOOTSEL** on your Pico 2.
2. Plug it into your PC.
3. Drag and drop [`04_emergency_monitor.uf2`](04_emergency_monitor.uf2) into the `RPI-RP2` drive.
