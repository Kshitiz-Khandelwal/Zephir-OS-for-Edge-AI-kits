# 15 — Task Communication (Keypad Producer/Consumer)

This program solves Experiment 13: *"Develop a system where a keypad task sends the entered key to another task that displays it on the LCD (or Serial Console)."*

## CONNECTION

| Component | Pico GPIO | Direction |
|-----------|-----------|-----------|
| **Keypad R1** | GP2 | Output |
| **Keypad R2** | GP3 | Output |
| **Keypad R3** | GP4 | Output |
| **Keypad R4** | GP5 | Output |
| **Keypad C1** | GP6 | Input (Pull-up) |
| **Keypad C2** | GP7 | Input (Pull-up) |
| **Keypad C3** | GP8 | Input (Pull-up) |
| **Keypad C4** | GP9 | Input (Pull-up) |

## 🧠 Concepts Explained: Producer/Consumer Pattern

This is a classic RTOS architectural pattern called **Producer-Consumer**.

### The Producer (Keypad Task)
The Producer thread is responsible for generating data. It continuously scans the 4x4 matrix keypad by driving rows LOW and reading the columns. When a button is physically pressed, it:
1. Locks the `keypad_mutex`.
2. Updates the shared variable `pressed_key`.
3. Unlocks the mutex.
4. Waits for the user to let go of the button (debounce).

### The Consumer (Display Task)
The Consumer thread does not care *how* the button was pressed or how a matrix keypad works. Its only job is to consume data. It continuously runs in the background (every 50ms):
1. Locks the `keypad_mutex` to read `pressed_key` safely.
2. Checks if the key is different from the `last_key` it printed.
3. If it's a new key, it prints `"Key Received: X"` to the Serial Console!

This separation of concerns is the hallmark of good embedded software design. The keypad driver and the display driver are completely isolated in their own threads, safely communicating via a Mutex-protected shared variable.

## Pre-compiled Firmware
You can flash this program directly to your board without building it yourself.
1. Hold **BOOTSEL** on your Pico 2.
2. Plug it into your PC.
3. Drag and drop [`15_keypad_communication.uf2`](15_keypad_communication.uf2) into the `RPI-RP2` drive.
