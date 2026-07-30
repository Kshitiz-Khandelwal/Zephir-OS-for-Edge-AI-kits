# 04 — Emergency Monitor (Non-Blocking Timers + Threads)

Demonstrates how to use `k_uptime_get_32()` to create non-blocking timers (similar to `time.ticks_ms()` in MicroPython) for the main thread, while a second thread monitors a sensor and interrupts normal operation with an emergency flash sequence if triggered.

## Circuit Connections

| Component | Pin | Raspberry Pi Pico |
|-----------|-----|-------------------|
| **LED Fast (+)** | - | GP14 |
| **LED Slow (+)** | - | GP15 |
| **LED Emergency (+)** | - | GP13 |
| **Touch Sensor (Output)** | - | GP16 |
| **Touch Sensor (VCC)** | VCC | 3.3V |
| **All LEDs & Sensor (-)** | GND | GND |

> **Note:** Use 330Ω resistors in series with each LED to prevent them from burning out!

## Pre-compiled Firmware
You can flash this program directly to your board without building it yourself.
1. Hold **BOOTSEL** on your Pico 2.
2. Plug it into your PC.
3. Drag and drop [`04_emergency_monitor.uf2`](04_emergency_monitor.uf2) into the `RPI-RP2` drive.
