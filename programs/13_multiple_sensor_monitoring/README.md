# 13 — Multiple Sensor Monitoring (ADC & Multitasking)

This program solves Experiment 11: *"Develop a multitasking embedded system using Zephyr RTOS that simultaneously monitors the Temperature Sensor, Light Sensor (LDR), and Joystick, displaying their values in real time using separate RTOS threads."*

## CONNECTION

| Sensor | Pico GPIO | ADC Channel |
|--------|-----------|-------------|
| **Light Sensor (LDR)** | GP26 | ADC0 |
| **Temperature Sensor** | GP27 | ADC1 |
| **Joystick X-axis** | GP28 | ADC2 |

*(Note: If you are using the IoT Sensor Board, these sensors are already internally connected. Do not connect any extra wires.)*

## 🧠 Concepts Explained: ADC & Independent Thread Scheduling

This project is the culmination of RTOS Multitasking. We have three physically distinct analog sensors, and we want to monitor them at completely different rates:
- **Temperature:** Every 1000 ms
- **Light:** Every 500 ms
- **Joystick:** Every 200 ms

### 1. Zephyr ADC API
In Zephyr, to read an analog voltage, we use the `adc_read()` API. 
We configured three independent `adc_channel_cfg` structures mapping directly to channels 0, 1, and 2. 
To ensure the hardware is fully aware of these channels, we also created an `app.overlay` DeviceTree file that defines `io-channels = <&adc 0>, <&adc 1>, <&adc 2>;`.

### 2. Independent Threading
Instead of a single `while(1)` loop trying to balance `sleep()` calls (which becomes a mathematical nightmare when you have 1000ms, 500ms, and 200ms overlapping), we simply spun up **three completely independent threads**:
- `temperature_task()`
- `light_task()`
- `joystick_task()`

Because they each have their own infinite loop and their own `k_sleep()` call, the Zephyr RTOS Kernel acts like an orchestra conductor. At exactly 200ms, it wakes up the Joystick thread. At 500ms, it wakes up both Light and Joystick threads. At 1000ms, it wakes up all three!

### Expected Output
When you flash this and open the Serial Monitor (via USB), you will see a continuous stream of data updating exactly at the requested intervals:
```
Multiple Sensor Monitoring Started
Joystick : 2048
Light : 1500
Joystick : 2050
Joystick : 2045
Light : 1520
Temperature : 24.5 C
```

## Pre-compiled Firmware
You can flash this program directly to your board without building it yourself.
1. Hold **BOOTSEL** on your Pico 2.
2. Plug it into your PC.
3. Drag and drop [`13_multiple_sensor_monitoring.uf2`](13_multiple_sensor_monitoring.uf2) into the `RPI-RP2` drive.
