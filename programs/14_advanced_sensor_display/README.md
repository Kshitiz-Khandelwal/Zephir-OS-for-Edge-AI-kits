# 14 — Advanced Sensor Display (ADC, TM1637, 4 Threads & Mutexes)

This program represents an advanced architectural upgrade to the Multiple Sensor Monitoring project. It adds a **TM1637 4-digit 7-segment display** to show the sensor readings directly on the board while simultaneously printing them to the Serial Console!

## CONNECTION

| Component | Pico GPIO | Notes |
|-----------|-----------|-------|
| **Light Sensor (LDR)** | GP26 (ADC0) | Connected internally on IoT Board |
| **Temperature Sensor** | GP27 (ADC1) | Connected internally on IoT Board |
| **Joystick X-axis** | GP28 (ADC2) | Connected internally on IoT Board |
| **TM1637 CLK** | GP3 | |
| **TM1637 DIO** | GP2 | |
| **TM1637 VCC** | 3V3(OUT) | Use 3.3 V so the display's CLK/DIO pull-ups never put 5 V on Pico GPIO. |
| **TM1637 GND** | GND | |

## 🧠 Concepts Explained: Shared Resources & Mutexes

This project elegantly combines everything we've learned about RTOS Architecture into a single capstone:

### 1. 4 Independent Threads
- **Temperature Thread:** Reads the sensor, updates the shared variable, prints to Serial every 1000ms.
- **Light Thread:** Reads the sensor, updates the shared variable, prints to Serial every 500ms.
- **Joystick Thread:** Reads the sensor, updates the shared variable, prints to Serial every 200ms.
- **Display Thread:** Reads all shared variables, updates the hardware TM1637 display, and cycles to the next sensor every 2000ms.

### 2. Protecting Shared Data with a Mutex
Because we have 3 threads writing data and 1 thread reading data, we have a **Shared Resource**:
```c
struct sensor_data_t {
	float temperature;
	int16_t light;
	int16_t joystick;
};
struct sensor_data_t shared_sensors;
```

To prevent a **Race Condition** (e.g., the Display thread trying to read the temperature at the exact microsecond the Temperature thread is overwriting it), we use an RTOS **Mutex** (`K_MUTEX_DEFINE(sensor_mutex)`). 

Before any thread touches `shared_sensors`, it must call `k_mutex_lock()`. Once it's done reading or writing, it calls `k_mutex_unlock()`. This ensures absolute data integrity.

### Expected Output
Immediately after flashing, the display must show `8888` for two seconds. This
is a hardware-only smoke test: if it does not appear, stop and check the four
TM1637 wires before checking any sensor code. Use **3V3(OUT)** for VCC, not
VBUS/5V; Pico GPIO pins are not 5 V tolerant.

On the **TM1637 Display**, the values automatically cycle every 2 seconds:
- `"t 29"` (Temperature) -> `"L512"` (Light) -> `"J 42"` (Joystick)

On the **Serial Console**, the raw data prints exactly at the specified intervals:
```
Temperature : 29.5 C
Light : 5123
Joystick : 421
Light : 5200
Joystick : 419
Joystick : 415
```

## Pre-compiled Firmware
You can flash this program directly to your board without building it yourself.
1. Hold **BOOTSEL** on your Pico 2.
2. Plug it into your PC.
3. Drag and drop [`14_advanced_sensor_display.uf2`](14_advanced_sensor_display.uf2) into the `RPI-RP2` drive.
