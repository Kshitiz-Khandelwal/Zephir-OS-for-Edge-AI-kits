# 10 — Software Timers (No Delays)

This program solves the RTOS exercise: *"Develop a reminder system that toggles an LED every five seconds without using delay loops."*

## CONNECTION

| Component | Pin | Raspberry Pi Pico |
|-----------|-----|-------------------|
| **LED (+)** | - | GP14 |
| **GND (-)** | GND | GND |

> **Note:** Use a 330Ω resistor in series with the LED to prevent it from burning out!

## 🧠 Concepts Explained: Software Timers vs Delays

### The Python Approach
In the Python code provided, the `toggle_flash` function was created and run on a separate CPU core (`_thread.start_new_thread`). Inside that thread, it used an infinite `while True` loop and `sleep(3)` to wait before toggling the LED again. 

While this works, creating an entire Thread (which requires allocating a dedicated Stack in RAM) just to blink an LED is incredibly wasteful in an embedded system!

### The RTOS Software Timer Approach
Zephyr RTOS provides **Software Timers** (`k_timer`). A Software Timer allows you to execute a function at a specific time, completely managed by the RTOS kernel in the background.

```c
void my_timer_expiry_function(struct k_timer *timer_id) {
    gpio_pin_toggle(gpio_dev, LED_PIN);
}

K_TIMER_DEFINE(my_timer, my_timer_expiry_function, NULL);
```

**Why this is vastly superior:**
1. **Zero Threads Required:** Notice that our `main()` function literally returns `0` and exits immediately! There is no `while (1)` loop anywhere in the C code!
2. **Zero Delay Loops:** There is no `k_sleep()` or `delay()` being called. 
3. **RAM Efficient:** We did not have to allocate a 1024-byte stack for a dedicated thread. 
4. **Accuracy:** The RTOS kernel tracks the hardware clock and calls `my_timer_expiry_function` exactly every 5 seconds.

This is the ultimate, most efficient way to run periodic, background "reminder" tasks!

## Pre-compiled Firmware
You can flash this program directly to your board without building it yourself.
1. Hold **BOOTSEL** on your Pico 2.
2. Plug it into your PC.
3. Drag and drop [`10_software_timers.uf2`](10_software_timers.uf2) into the `RPI-RP2` drive.
