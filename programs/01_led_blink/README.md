# 01 — 3-Color LED Blink

A simple introductory Zephyr RTOS program that sequentially blinks three external LEDs. Converted from a MicroPython example to bare-metal C.

## Circuit Connections

| LED | Pin | Raspberry Pi Pico |
|-----|-----|-------------------|
| **Red LED (+)** | - | GP0 |
| **Green LED (+)** | - | GP1 |
| **Blue LED (+)** | - | GP2 |
| **All LEDs (-)** | GND | GND |

> **Note:** Use 330Ω resistors in series with each LED to prevent them from burning out!

## Pre-compiled Firmware
You can flash this program directly to your board without building it yourself.
1. Hold **BOOTSEL** on your Pico 2.
2. Plug it into your PC.
3. Drag and drop [`01_led_blink.uf2`](01_led_blink.uf2) into the `RPI-RP2` drive.
