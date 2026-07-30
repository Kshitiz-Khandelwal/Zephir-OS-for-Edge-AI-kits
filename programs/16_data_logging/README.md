# 16 — Data Logging (ADC & Mutex Flags)

This program solves Experiment 14: *"Design a system that records sensor readings every second while continuously displaying the latest reading on the screen."*

## CONNECTION

| Component | Pico GPIO | Notes |
|-----------|-----------|-------|
| **Light Sensor (LDR)** | GP26 (ADC0) | Connected internally on IoT Board |

## 🧠 Concepts Explained: Status Flags & Mutexes

This project demonstrates how to safely pass data between two threads running at vastly different speeds.

### The Logger Thread (1000ms)
The Logger task is responsible for interacting with the hardware. It reads the LDR sensor (via the Zephyr ADC API) once per second. When it gets a new reading, it safely locks a mutex, updates a global variable `latest_reading`, sets a boolean flag `new_data = true`, and unlocks the mutex.

### The Display Thread (100ms)
The Display task needs to be extremely responsive, so it loops 10 times faster than the Logger (every 100ms). However, instead of redundantly printing the same data 10 times, it simply checks the `new_data` flag!
- If `new_data` is true: It prints the value and resets the flag to false.
- If `new_data` is false: It does nothing.

This architecture ensures the Display thread is always instantly ready to show new data, without wasting CPU cycles processing stale data!

## Pre-compiled Firmware
You can flash this program directly to your board without building it yourself.
1. Hold **BOOTSEL** on your Pico 2.
2. Plug it into your PC.
3. Drag and drop [`16_data_logging.uf2`](16_data_logging.uf2) into the `RPI-RP2` drive.
