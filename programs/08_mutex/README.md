# 08 — Mutexes & Resource Protection

This program solves the RTOS exercise: *"Develop a system where two different tasks attempt to write information to the same LCD display without corrupting the output."*

## CONNECTION

| Component | Pin | Raspberry Pi Pico |
|-----------|-----|-------------------|
| **TM1637 CLK** | CLK | GP16 |
| **TM1637 DIO** | DIO | GP17 |
| **TM1637 VCC** | VCC | 5V (VBUS) |
| **TM1637 GND** | GND | GND |

## 🧠 Concepts Explained: Mutex (Mutual Exclusion)

A **Mutex** (Mutual Exclusion) is an RTOS lock used to protect a shared resource from being accessed by multiple threads at the exact same time.

### The Scenario
We have two threads in this program:
1. **Sensor Thread:** Wants to write `"DATA"` to the screen every 1.5 seconds.
2. **Main Thread:** Wants to write `"PING"` to the screen every 1.0 second.

The TM1637 display uses a custom bit-banged communication protocol. Sending data to it takes time (setting pins High/Low repeatedly). 

### The Problem
If the Main Thread is halfway through sending the word `"PING"`, and the Sensor Thread suddenly wakes up and tries to send `"DATA"` at the exact same millisecond, their electrical signals on the DIO and CLK pins will collide! The display will get corrupted garbage data.

### How the Mutex Works
We created `display_mutex`. Before any thread can talk to the display, it MUST acquire the lock:
```c
k_mutex_lock(&display_mutex, K_FOREVER);
```

- When the **Main Thread** wants to say `"PING"`, it locks the Mutex.
- If the **Sensor Thread** wakes up a millisecond later and wants to say `"DATA"`, it tries to lock the Mutex. But it can't! The RTOS immediately puts the Sensor Thread back to **Sleep** (`K_FOREVER`) because the Mutex is currently held by the Main Thread.
- Once the **Main Thread** finishes sending the entire word `"PING"`, it calls `k_mutex_unlock(&display_mutex)`.
- The RTOS instantly wakes up the **Sensor Thread**, gives it the lock, and allows it to safely send `"DATA"`.

This guarantees that our bit-banged output sequence is never interrupted or corrupted by another thread!

## Pre-compiled Firmware
You can flash this program directly to your board without building it yourself.
1. Hold **BOOTSEL** on your Pico 2.
2. Plug it into your PC.
3. Drag and drop [`08_mutex.uf2`](08_mutex.uf2) into the `RPI-RP2` drive.
