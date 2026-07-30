# 05 — TM1637 Display & Heartbeat (Bit-Banging)

This program demonstrates how to control a TM1637 4-digit 7-segment display using **bit-banging** (software I/O operations) alongside a non-blocking heartbeat LED timer.

## CONNECTION

**TM1637 Display:**

| TM1637 Pin | Raspberry Pi Pico |
|:---|:---|
| **CLK** | GP16 |
| **DIO** | GP17 |
| **VCC** | 5V (VBUS) |
| **GND** | GND |

**Heartbeat LED:**

| LED | Pico GPIO |
|:---|:---|
| **Heartbeat (+)** | GP14 |
| **GND (-)** | GND |

> **Note:** Use a 330Ω resistor in series with the Heartbeat LED to prevent it from burning out!

## 🧠 Concepts Explained

### 1. I/O Operations System (Bit-Banging vs Peripherals)
The Raspberry Pi Pico has hardware peripherals (like I2C, SPI, UART) that handle communication automatically in the background. However, the TM1637 uses a custom protocol that isn't standard I2C. 

To solve this, we use **Bit-Banging**. This means we manually toggle the GPIO pins HIGH and LOW in our code to simulate the protocol's clock and data lines. 
- `gpio_pin_set(dev, CLK_PIN, 1)` and `k_busy_wait(20)` are used to manually create the tiny microsecond-level square waves needed to talk to the chip.
- It requires precise timing but allows you to connect the display to *any* general-purpose pins.

### 2. Non-Blocking Multitasking (Without Threads)
Unlike Program 03 & 04 which used RTOS Threads, this program runs everything in a single `main` loop using state machines.
- We read the current time using `k_uptime_get_32()`.
- We check if enough time has passed: `(current_time - last_time) >= INTERVAL`.
- If yes, we execute the task (update display or flash LED) and save the new time.
- If no, we simply skip and `k_sleep(K_MSEC(10))` to yield the CPU.
This prevents a `sleep(1)` command from freezing the entire board, allowing the heartbeat LED and the display counter to run simultaneously.

## Pre-compiled Firmware
You can flash this program directly to your board without building it yourself.
1. Hold **BOOTSEL** on your Pico 2.
2. Plug it into your PC.
3. Drag and drop [`05_tm1637_display.uf2`](05_tm1637_display.uf2) into the `RPI-RP2` drive.
