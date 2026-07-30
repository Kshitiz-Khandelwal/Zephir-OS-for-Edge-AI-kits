# Sleep and Delay in Embedded Systems

## The Problem
Embedded systems often have to perform multiple tasks simultaneously.

For example:
- Blink an LED every 1 second (Heartbeat Indicator)
- Update an LCD every 500 ms
- Read a sensor every 100 ms
- Check whether a button is pressed

Although these tasks appear to run at the same time, a microcontroller usually has one CPU core, meaning it can execute only one instruction at a time. Therefore, the processor must schedule its time efficiently among all tasks.

The challenge is ensuring that waiting for one task does not prevent other tasks from executing. This is where **Delay** and **Sleep** come into the picture.

---

## What is Delay?
A delay is a function that intentionally pauses program execution for a specified period.

**Example:**
```c
LED_ON();
delay(1000);
LED_OFF();
```

Here:
1. Turn LED ON
2. Wait 1000 ms
3. Turn LED OFF

During the 1000 ms delay, the processor does not execute the next statements in the program.

Depending on the platform, the CPU may either:
- Continuously execute an empty loop (busy waiting), or
- Enter a temporary wait until the delay expires.

From the application's perspective, the program is **blocked** because it cannot continue to the next instruction until the delay completes.

### Timeline of delay()
```text
Time (ms)
0                500              1000
LED ON  -------------------------> LED OFF

Program State
Executing ---- Waiting ---- Waiting ---- Continue
```
*Notice that nothing after `delay()` executes until 1000 ms have elapsed.*

---

## What is Blocking?
A blocking operation prevents the program from performing other work until the current operation finishes.

Suppose we have two tasks:
- Blink LED every 1000 ms
- Update Display every 500 ms

If we write:
```c
while(1) {
    blinkLED();
    delay(1000);
    updateDisplay();
}
```

### Timeline
```text
0 ms
Blink LED
   ↓
Wait 1000 ms
   ↓
Update Display
```

**Problem:**
The display should update at 500 ms, 1000 ms, 1500 ms...
Instead, it updates only *after* every delay. This means one task blocks another, causing inaccurate timing and poor responsiveness.

---

## What is Sleep?
Sleep is different from delay. Instead of simply waiting, the processor enters a **low-power operating mode**.

When the processor sleeps:
- CPU clock is reduced or stopped.
- Most instruction execution halts.
- Power consumption decreases significantly.
- Certain peripherals (timers, interrupts, RTC) remain active.

The processor wakes when:
- A timer expires
- An interrupt occurs
- An external event happens

**For example:**
```c
sleep(1000);
```

**Conceptually:**
```text
Processor Running
       ↓
Enter Sleep Mode
       ↓
Timer Counts
       ↓
Wake Up
       ↓
Resume Execution
```

Unlike a busy delay, sleep is designed primarily for power efficiency, which is crucial in battery-powered devices like smartwatches, IoT sensors, and wearables.

### Delay vs Sleep
| Feature | Delay | Sleep |
|---------|-------|-------|
| **Purpose** | Wait for a fixed time | Reduce power consumption while waiting |
| **CPU State** | Active (or blocked in software) | Low-power mode |
| **Power Consumption** | Higher | Much lower |
| **Used In** | Simple applications | Battery-powered systems |
| **Wake-up** | After delay expires | Timer or interrupt |

---

## Why Delay Becomes a Problem
Suppose your embedded system performs these operations:
- **Task A**: Blink LED every 1 sec
- **Task B**: Read Temperature every 100 ms
- **Task C**: Update LCD every 500 ms
- **Task D**: Send WiFi Data every 2 sec

If Task A contains `delay(1000);`, then:
- Temperature is not read during that second.
- LCD updates are postponed.
- WiFi transmission is delayed.

One blocking delay negatively affects every other task. This makes the system less responsive.

---

## Better Solution: Non-Blocking Timing
Instead of asking *"Wait for 1000 ms"*, we ask *"Has 1000 ms elapsed?"*

This is known as **non-blocking timing**.

**Pseudo-code:**
```c
while(1) {
    currentTime = getCurrentTime();

    if (currentTime - lastLEDTime >= 1000) {
        blinkLED();
        lastLEDTime = currentTime;
    }

    if (currentTime - lastDisplayTime >= 500) {
        updateDisplay();
        lastDisplayTime = currentTime;
    }
}
```

Notice that the processor never stops running. Instead, it continuously checks whether each task is due.

### Execution Timeline
```text
Time →
0      500      1000     1500     2000

LED
ON               OFF                ON

Display
Update   Update   Update   Update   Update
```
Here, the display updates every 500 ms and the LED changes every 1000 ms. Neither task blocks the other.

### Why This Works
The processor loops extremely fast—often thousands or millions of times per second.
Each iteration checks:
- Is it time to blink? -> No
- Is it time to update display? -> Yes -> Update display
- Continue looping

Since each task executes only when required, the CPU remains available for other work between events.

---

## Relation to RTOS
In an RTOS (Real-Time Operating System), this scheduling is handled automatically.

You can create separate tasks:
- **Task 1:** Blink LED every 1 second
- **Task 2:** Update LCD every 500 ms
- **Task 3:** Read Sensor every 100 ms

The RTOS scheduler allocates CPU time to each task according to priority and timing requirements. Instead of using `delay()`, RTOS APIs often provide functions like `vTaskDelay()` (or `k_sleep()` in Zephyr), which suspend only the current task, allowing other tasks to continue running!
