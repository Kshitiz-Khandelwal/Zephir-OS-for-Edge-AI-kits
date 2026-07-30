# 11 — GPIO Interrupts (Hardware ISR)

This program solves the RTOS exercise: *"Design a system that instantly responds to a button press while background tasks continue running normally."*

## CONNECTION

| Component | Pin | Raspberry Pi Pico |
|-----------|-----|-------------------|
| **Button** | OUT | GP2 (Internal Pull-Up) |
| **Button** | GND | GND |
| **LED (+)** | - | GP14 |
| **GND (-)** | GND | GND |

> **Note:** Use a 330Ω resistor in series with the LED to prevent it from burning out!

## 🧠 Concepts Explained: Interrupts vs Polling

### The Python Approach (Polling)
In the Python code provided, the main loop did this:
```python
while True:
    if touch.value():
        print("Touched")
    sleep(0.2)
```
This is called **Polling**. The CPU wakes up every 0.2 seconds (200ms) and checks the pin. 

**The fatal flaw:** If the user presses and releases the button *really fast* exactly in the middle of that 200ms sleep, the system will completely miss the button press! The event is lost forever because the CPU was sleeping.

### The RTOS Hardware Interrupt Approach
In Zephyr, we use the microcontroller's hardware **Interrupt Service Routine (ISR)**.

```c
gpio_pin_interrupt_configure(gpio_dev, BUTTON_PIN, GPIO_INT_EDGE_TO_ACTIVE);
gpio_init_callback(&button_cb_data, button_pressed_isr, BIT(BUTTON_PIN));
```

**How it works:**
1. Our Main Thread has a background task that prints `"="` and then goes to sleep for **800ms**.
2. If we were using polling, a button press during that 800ms sleep would be ignored.
3. However, because we configured a hardware interrupt, the physical electrical change on the GP2 pin bypasses the operating system entirely and sends a direct signal to the CPU hardware!
4. The CPU instantly halts the main thread (even if it's sleeping!), jumps directly to the `button_pressed_isr()` function, turns on the LED, prints `*** INTERRUPT ***`, and then returns exactly back to what it was doing.

This guarantees absolute **zero-latency response** to the button press, and guarantees that no events are ever missed, no matter how heavy or slow the background tasks are!

## Pre-compiled Firmware
You can flash this program directly to your board without building it yourself.
1. Hold **BOOTSEL** on your Pico 2.
2. Plug it into your PC.
3. Drag and drop [`11_gpio_interrupt.uf2`](11_gpio_interrupt.uf2) into the `RPI-RP2` drive.
