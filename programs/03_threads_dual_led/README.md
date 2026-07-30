# 03 — Dual LED Blink with RTOS Threads

Demonstrates how to use multiple independent execution threads in Zephyr RTOS (`K_THREAD_DEFINE`) to do two things at once — blinking two LEDs at completely different rates.

## Circuit Connections

| LED | Pin | Raspberry Pi Pico |
|-----|-----|-------------------|
| **LED 1 (+)** | - | GP14 |
| **LED 2 (+)** | - | GP15 |
| **All LEDs (-)** | GND | GND |

> **Note:** Use 330Ω resistors in series with each LED to prevent them from burning out!

## Pre-compiled Firmware
You can flash this program directly to your board without building it yourself.
1. Hold **BOOTSEL** on your Pico 2.
2. Plug it into your PC.
3. Drag and drop [`03_threads_dual_led.uf2`](03_threads_dual_led.uf2) into the `RPI-RP2` drive.
