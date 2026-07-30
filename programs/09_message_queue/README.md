# 09 — Message Queues (Data Passing)

This program solves the RTOS exercise: *"Design a system where one task reads temperature values and another task displays them on the LCD."*

## CONNECTION

| Component | Pin | Raspberry Pi Pico |
|-----------|-----|-------------------|
| **TM1637 CLK** | CLK | GP16 |
| **TM1637 DIO** | DIO | GP17 |
| **TM1637 VCC** | VCC | 5V (VBUS) |
| **TM1637 GND** | GND | GND |

## 🧠 Concepts Explained: Message Queues

In embedded systems, tasks often need to share **data** (like a sensor reading) with other tasks. While global variables *can* be used, they are incredibly dangerous in an RTOS because a task might read a variable while another task is halfway through writing to it.

A **Message Queue** is an RTOS feature that safely transfers data from one thread to another.

### The Scenario
1. **Sensor Task (Producer):** Reads the temperature (we simulated this to save wiring) and generates a value (e.g., `25`).
2. **Display Task (Consumer):** Updates the TM1637 display with the temperature.

### How it Works
We defined a Message Queue that holds integers:
```c
K_MSGQ_DEFINE(temp_msgq, sizeof(int), 10, 4);
```

1. The Sensor Task gets the data and uses `k_msgq_put()` to instantly drop the data into the queue.
2. The Display Task uses `k_msgq_get(..., K_FOREVER)`. This means the Display Task goes to **Sleep** and does absolutely nothing, consuming 0 CPU power, until data arrives.
3. The moment the Sensor Task puts data in the queue, the RTOS wakes up the Display Task and hands it the data. It then bit-bangs the TM1637 to show `"25 C"`.

**Benefits over Python code:**
The Python script used a global variable `temperature` and `sleep()` polling loops in both threads. By using a Message Queue, we completely eliminate polling and guarantee data safety!

## Pre-compiled Firmware
You can flash this program directly to your board without building it yourself.
1. Hold **BOOTSEL** on your Pico 2.
2. Plug it into your PC.
3. Drag and drop [`09_message_queue.uf2`](09_message_queue.uf2) into the `RPI-RP2` drive.
