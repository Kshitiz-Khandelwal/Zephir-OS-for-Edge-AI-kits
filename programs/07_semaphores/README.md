# 07 — Semaphores & Cross-Thread Signaling

This program solves the RTOS exercise: *"Design a system where pressing a push button allows another task to update the LCD with a message."*

## CONNECTION

| Component | Pin | Raspberry Pi Pico |
|-----------|-----|-------------------|
| **TM1637 CLK** | CLK | GP16 |
| **TM1637 DIO** | DIO | GP17 |
| **TM1637 VCC** | VCC | 5V (VBUS) |
| **TM1637 GND** | GND | GND |
| **Joystick Button** | SW | GP22 (Internal Pull-Up) |
| **Joystick GND** | GND | GND |

## 🧠 Concepts Explained: Semaphores

A **Semaphore** is an RTOS tool used for synchronization between threads. You can think of a semaphore as a "token" or a "signal".

### The Scenario
We have two threads in this program:
1. **Input Thread:** Checks the Joystick button on GP22.
2. **Display Thread:** Updates the TM1637 4-digit display.

We want the Display Thread to show `"OPEN"` when the button is held, and `"LOC "` (Lock) when it is released.

### How the Semaphore Works
The Display Thread sits and **waits** for a semaphore using `k_sem_take()`. We gave it a timeout of 100 milliseconds. 

- The Input Thread checks the button every 50ms.
- **If the button is pressed:** The Input Thread "gives" the semaphore (`k_sem_give()`). The Display Thread instantly wakes up, sees that it successfully took the semaphore, and prints `"OPEN"`.
- **If the button is NOT pressed:** The Input Thread does nothing. The Display Thread waits 100ms, realizes no semaphore was given (a "timeout"), and prints `"LOC "`.

**Why is this better than global variables?**
If we just used a boolean variable `bool is_pressed`, the Display Thread would have to constantly spin in a tight loop checking `if (is_pressed)`. This wastes massive amounts of CPU power. By using a semaphore, the Display Thread goes to **Sleep** and the RTOS only wakes it up when the signal is actually received!

## Pre-compiled Firmware
You can flash this program directly to your board without building it yourself.
1. Hold **BOOTSEL** on your Pico 2.
2. Plug it into your PC.
3. Drag and drop [`07_semaphores.uf2`](07_semaphores.uf2) into the `RPI-RP2` drive.
