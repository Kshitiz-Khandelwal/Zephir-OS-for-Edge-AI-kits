# 03 — Dual LED Blink with RTOS Threads

Demonstrates how to use multiple independent execution threads in Zephyr RTOS (`K_THREAD_DEFINE`) to do two things at once — blinking two LEDs at completely different rates.

## Circuit Connections

| LED | Pin | Raspberry Pi Pico |
|-----|-----|-------------------|
| **LED 1 (+)** | - | GP14 |
| **LED 2 (+)** | - | GP15 |
| **All LEDs (-)** | GND | GND |

> **Note:** Use 330Ω resistors in series with each LED to prevent them from burning out!

## 🧠 Concepts Explained

### The Problem: Blocking Delays
In standard embedded programming, a `delay(1000)` or `sleep(1)` function stops the CPU from executing the next statement. If you want to blink an LED every 1000ms and update a display every 500ms, a blocking delay prevents the second task from running accurately. One task blocks another, causing poor responsiveness.

### The Solution: RTOS Threads
In a Real-Time Operating System (RTOS) like Zephyr, this scheduling is handled automatically by the OS. Instead of running a single `main` loop, you create separate tasks (threads):
- **Thread 1:** Blinks LED 1 every 500 ms
- **Thread 2:** Blinks LED 2 every 200 ms

The RTOS scheduler automatically allocates CPU time to each task based on priority. When we use `k_sleep()` (unlike a busy loop or normal delay), we tell the RTOS to put the current thread to **Sleep** (a low-power or blocked state) and immediately switch the CPU to execute the other thread. 

This means multiple tasks appear to run simultaneously without interfering with each other!

## Pre-compiled Firmware
You can flash this program directly to your board without building it yourself.
1. Hold **BOOTSEL** on your Pico 2.
2. Plug it into your PC.
3. Drag and drop [`03_threads_dual_led.uf2`](03_threads_dual_led.uf2) into the `RPI-RP2` drive.
