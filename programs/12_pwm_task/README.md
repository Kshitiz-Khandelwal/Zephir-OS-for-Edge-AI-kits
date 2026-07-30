# 12 — PWM Task (Concurrent Fading)

This program solves the RTOS exercise: *"Design a system that continuously changes the brightness of an RGB LED while another task displays system status."*

## CONNECTION

| Component | Pin | Raspberry Pi Pico |
|-----------|-----|-------------------|
| **Red LED** | OUT | GP15 (+ 330Ω resistor) to GND |
| **Green LED** | OUT | GP14 (+ 330Ω resistor) to GND |
| **Blue LED** | OUT | GP13 (+ 330Ω resistor) to GND |

## 🧠 Concepts Explained: Software PWM & Preemption

In Zephyr RTOS, hardware PWM is incredibly powerful, but to configure it requires modifying the complex DeviceTree. To keep this exercise focused on **Threading** and **Concurrency** (which is the goal of this exercise), we are using a **Software PWM** loop.

### How Software PWM Works
PWM (Pulse Width Modulation) is just turning a pin ON and OFF really fast. If you do it fast enough, the human eye perceives it as changing brightness.
Our main thread runs a loop with a 2ms period (500Hz).
1. It turns the LED **ON**.
2. It waits for `duty` microseconds.
3. It turns the LED **OFF**.
4. It waits for the remainder of the 2ms period.

By constantly changing the `duty` value, the LED fades in and out!

### Concurrency and Preemption
Wait, if our main thread is constantly running a rapid 500Hz while loop to fade the LED, how can the System Status thread ever run? Won't it block the CPU?

This is the magic of the **Zephyr RTOS Preemptive Scheduler**:
1. We gave both the Status thread and the Main thread the same priority.
2. The Main thread calls `k_yield()` at the end of its 2ms loop, which explicitly tells the OS: *"I'm done for a fraction of a millisecond, let anyone else run if they need to."*
3. The Status thread is asleep most of the time (`k_sleep(1000)`). But exactly every 1 second, it wakes up.
4. Because of preemption and yielding, the OS seamlessly pauses the PWM loop for a few microseconds, lets the Status thread print `"System Status : Running"`, and immediately resumes the PWM loop.

The LED fades flawlessly without flickering, and the status prints exactly on time!

## Pre-compiled Firmware
You can flash this program directly to your board without building it yourself.
1. Hold **BOOTSEL** on your Pico 2.
2. Plug it into your PC.
3. Drag and drop [`12_pwm_task.uf2`](12_pwm_task.uf2) into the `RPI-RP2` drive.
