# 17 — Producer-Consumer Model (Potentiometer)

This program solves Experiment 9: *"Develop a system where one task continuously samples a potentiometer and another task processes and displays the readings."*

## CONNECTION

| Component | Pico GPIO | Notes |
|-----------|-----------|-------|
| **Potentiometer** | GP26 (ADC0) | Connected internally on IoT Board |

## 🧠 Concepts Explained: Producer-Consumer with ADC

This project is the quintessential RTOS implementation of the Producer-Consumer architectural pattern applied to an analog sensor:

### Task 1: The Producer (Sampling Task)
This task interacts directly with the Zephyr hardware driver for the ADC.
1. It requests an analog reading from GP26.
2. It mathematically scales the 12-bit hardware value (0-4095) into a 16-bit software value (0-65535) to match standard Python outputs.
3. It locks the RTOS Mutex (`lock`), copies the data to a shared variable `pot_value`, and flags `new_data = true`.
4. It unlocks the Mutex and yields the CPU (sleeps) for 100ms.

### Task 2: The Consumer (Processing Task)
This task never touches the hardware. It strictly consumes data produced by Task 1.
1. It locks the Mutex and checks the `new_data` flag.
2. If new data exists, it safely copies it to a local variable and resets the flag.
3. It performs processing: converting the raw 16-bit reading into a human-readable percentage (0% to 100%).
4. It prints the processed results beautifully to the USB Serial Console.

By separating the "Hardware Sampling" from the "Data Processing", you create code that is robust, modular, and easy to upgrade!

## Pre-compiled Firmware
1. Hold **BOOTSEL** on your Pico 2.
2. Plug it into your PC.
3. Drag and drop [`17_producer_consumer_potentiometer.uf2`](17_producer_consumer_potentiometer.uf2) into the `RPI-RP2` drive.
4. Open your Serial Monitor (e.g. Thonny) to view the percentages!
