# ⚡ Zephyr RTOS for Edge AI IoT Kits
### Raspberry Pi Pico 2 (RP2350) — Complete Setup Guide

> A collection of Zephyr RTOS programs for IoT and Edge AI projects on the Raspberry Pi Pico 2.
> From basic GPIO to advanced AI-powered anomaly detection — all on a $5 microcontroller.

---

## 📋 Table of Contents

- [What is This?](#what-is-this)
- [Hardware Required](#hardware-required)
- [Step 1 — Install Prerequisites](#step-1--install-prerequisites)
- [Step 2 — Install West & ARM Toolchain](#step-2--install-west--arm-toolchain)
- [Step 3 — Set Up Zephyr Workspace](#step-3--set-up-zephyr-workspace)
- [Step 4 — Build a Program](#step-4--build-a-program)
- [Step 5 — Flash to Your Board](#step-5--flash-to-your-board)
- [Programs in This Repo](#programs-in-this-repo)
- [How to Flash Every Time](#how-to-flash-every-time)
- [Troubleshooting](#troubleshooting)

---

## What is This?

This repository contains **ready-to-build Zephyr RTOS programs** for the Raspberry Pi Pico 2 (RP2350).

**Why Zephyr over MicroPython or Arduino?**

| Feature | MicroPython | Arduino | Zephyr RTOS |
|---------|-------------|---------|-------------|
| Real-time scheduling | ❌ | ❌ | ✅ |
| Multi-threading | ❌ | ❌ | ✅ |
| Wi-Fi + BLE stack | Limited | Limited | ✅ Built-in |
| Edge AI (TFLite) | ❌ | Limited | ✅ |
| Production-ready | ❌ | Limited | ✅ |
| Best for IoT | ⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ |

---

## Hardware Required

| Item | Required For |
|------|-------------|
| Raspberry Pi Pico 2 (RP2350) | All programs |
| Micro USB cable (data, not charge-only!) | All programs |
| LEDs + 330Ω resistors | LED programs |
| L298N Motor Driver Module | DC Motor program |
| DC Motor (5-12V) | DC Motor program |
| External power supply (6-12V) | DC Motor program |
| Breadboard + jumper wires | All programs |

---

## Step 1 — Install Prerequisites

> ✅ Already have these? Check with the verify commands and skip ahead.

### 1.1 — Python 3.12+
Download from [python.org](https://www.python.org/downloads/)

```powershell
python --version   # should show 3.12 or higher
```

### 1.2 — Git
Download from [git-scm.com](https://git-scm.com/download/win)

```powershell
git --version
```

### 1.3 — CMake
Download from [cmake.org](https://cmake.org/download/)

```powershell
cmake --version   # should show 3.20 or higher
```

### 1.4 — Ninja Build
Download from [ninja-build.org](https://ninja-build.org/) and add to PATH

```powershell
ninja --version
```

---

## Step 2 — Install West & ARM Toolchain

### 2.1 — Install West (Zephyr's build tool)

```powershell
pip install west
```

Add to PATH permanently (replace `Python314` with your version):
```powershell
[System.Environment]::SetEnvironmentVariable("PATH",
  $env:PATH + ";C:\Users\$env:USERNAME\AppData\Roaming\Python\Python314\Scripts",
  [System.EnvironmentVariableTarget]::User)
```

Verify:
```powershell
west --version   # should show west 1.x.x
```

### 2.2 — Download ARM GNU Toolchain

Download the **ARM GNU Toolchain 14.x for Windows** (zip version):
👉 [ARM Developer Downloads](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)

Choose: `arm-gnu-toolchain-14.x.rel1-mingw-w64-i686-arm-none-eabi.zip`

Extract to `C:\arm-toolchain`

Add to PATH permanently:
```powershell
[System.Environment]::SetEnvironmentVariable("PATH",
  [System.Environment]::GetEnvironmentVariable("PATH", "User") + ";C:\arm-toolchain\bin",
  [System.EnvironmentVariableTarget]::User)
```

Verify:
```powershell
arm-none-eabi-gcc --version   # should show gcc 14.x
```

---

## Step 3 — Set Up Zephyr Workspace

> ⚠️ This downloads ~2-3 GB. Make sure you have space and a good internet connection.

### 3.1 — Initialize workspace

```powershell
west init C:\zephyrproject
cd C:\zephyrproject
```

### 3.2 — Download all modules (~2-3 GB, takes 15-30 min)

```powershell
west update
```

### 3.3 — Install Python dependencies

```powershell
pip install -r C:\zephyrproject\zephyr\scripts\requirements.txt
C:\Python314\python.exe -m pip install pyelftools
```
> Replace `Python314` with your actual Python version folder name.

### 3.4 — Export Zephyr to CMake

```powershell
cd C:\zephyrproject\zephyr
west zephyr-export
```

### 3.5 — Fetch Wi-Fi firmware blobs (for Pico W/Pico 2 W)

```powershell
cd C:\zephyrproject
west blobs fetch hal_infineon
```

---

## Step 4 — Build a Program

### Set environment variables (run once per PowerShell session)

```powershell
$env:PATH += ";C:\Users\$env:USERNAME\AppData\Roaming\Python\Python314\Scripts;C:\arm-toolchain\bin"
$env:ZEPHYR_TOOLCHAIN_VARIANT = "gnuarmemb"
$env:GNUARMEMB_TOOLCHAIN_PATH = "C:\arm-toolchain"
```

### Clone this repository

```powershell
git clone https://github.com/Kshitiz-Khandelwal/Zephir-OS-for-Edge-AI-kits.git
cd Zephir-OS-for-Edge-AI-kits
```

### Build any program

```powershell
# Example: LED Blink
west build -b rpi_pico2/rp2350a/m33 programs\01_led_blink

# Example: DC Motor
west build -b rpi_pico2/rp2350a/m33 programs\02_dc_motor
```

The compiled firmware will be at:
```
build\zephyr\zephyr.uf2
```

---

## Step 5 — Flash to Your Board

### Method: UF2 Drag & Drop (easiest!)

```
1. Hold the BOOTSEL button on your Pico 2
2. While holding, plug the USB cable into your PC
3. Release the BOOTSEL button
4. A drive called "RPI-RP2" appears in File Explorer
5. Copy build\zephyr\zephyr.uf2 to the RPI-RP2 drive
6. The drive disappears → board reboots → program runs!
```

> ✅ That's it! No special flasher software needed.

---

## Programs in This Repo

### 01 — 3-Color LED Blink

**Folder:** `programs/01_led_blink/`

**Wiring:**
```
Pico 2 GP0  →  [330Ω]  →  RED LED (+)    →  GND
Pico 2 GP1  →  [330Ω]  →  GREEN LED (+)  →  GND
Pico 2 GP2  →  [330Ω]  →  BLUE LED (+)   →  GND
```

**What it does:** Blinks all 3 LEDs ON for 1 second, OFF for 1 second.

**Build:**
```powershell
west build -b rpi_pico2/rp2350a/m33 programs\01_led_blink
```

---

### 02 — DC Motor Control (L298N)

**Folder:** `programs/02_dc_motor/`

**Wiring:**
```
Pico 2 GP2  →  L298N IN1     (direction)
Pico 2 GP3  →  L298N IN2     (direction)
Pico 2 GP4  →  L298N ENA     (speed via PWM) ← remove jumper!
Pico 2 GND  →  L298N GND     (COMMON GROUND — required!)

L298N OUT1  →  Motor +
L298N OUT2  →  Motor −

External 6-12V battery  →  L298N 12V & GND
```

**What it does:**
1. Forward at 50% speed (3 sec)
2. Ramps to 100% speed (1.5 sec)
3. Stop (1 sec)
4. Backward at 75% (3 sec)
5. Repeat

**Build:**
```powershell
west build -b rpi_pico2/rp2350a/m33 programs\02_dc_motor
```

---

### 03 — Dual LED Blink with RTOS Threads

**Folder:** `programs/03_threads_dual_led/`

**Wiring:**
```
Pico 2 GP14  →  [330Ω]  →  LED1 (+)  →  GND   (blinks every 500 ms)
Pico 2 GP15  →  [330Ω]  →  LED2 (+)  →  GND   (blinks every 200 ms)
```

**What it does:**
- Thread 1 (main): toggles LED1 every **500 ms**
- Thread 2 (spawned via `K_THREAD_DEFINE`): toggles LED2 every **200 ms**
- Both run **simultaneously** — demonstrating true RTOS multithreading

**MicroPython → Zephyr translation:**

| MicroPython | Zephyr RTOS |
|-------------|-------------|
| `_thread.start_new_thread(fn, ())` | `K_THREAD_DEFINE(name, stack, fn, ...)` |
| `time.sleep(0.5)` | `k_sleep(K_MSEC(500))` |
| `led.toggle()` | `gpio_pin_toggle(dev, pin)` |

**Build:**
```powershell
west build -b rpi_pico2/rp2350a/m33 programs\03_threads_dual_led
```

---

### 04 — Emergency Monitor & Non-Blocking Timers

**Folder:** `programs/04_emergency_monitor/`

**Wiring:**
```
Pico 2 GP14  →  [330Ω]  →  LED_FAST (+)      →  GND
Pico 2 GP15  →  [330Ω]  →  LED_SLOW (+)      →  GND
Pico 2 GP13  →  [330Ω]  →  LED_EMERGENCY (+) →  GND
Pico 2 GP16  →  Touch Sensor / Button        →  3.3V (VCC)
```

**What it does:**
- Thread 1 (Main Loop):
  - Uses `k_uptime_get_32()` to check the time non-blockingly.
  - Toggles `LED_FAST` every **200 ms**.
  - Toggles `LED_SLOW` every **1000 ms**.
- Thread 2 (Emergency Monitor):
  - Continuously checks the sensor on `GP16`.
  - If triggered (goes HIGH), it interrupts its normal wait and rapidly flashes `LED_EMERGENCY` 5 times with a 50ms delay.

**MicroPython → Zephyr translation:**

| MicroPython | Zephyr RTOS |
|-------------|-------------|
| `time.ticks_ms()` | `k_uptime_get_32()` |
| `time.ticks_diff(c, l)` | `current_time - last_time` |
| `Pin(16, IN, PULL_DOWN)` | `gpio_pin_configure(..., GPIO_INPUT \| GPIO_PULL_DOWN)` |

**Build:**
```powershell
west build -b rpi_pico2/rp2350a/m33 programs\04_emergency_monitor
```

---

## How to Flash Every Time

Each time you want to flash a NEW program:

```powershell
# Step 1: Set environment (once per session)
$env:PATH += ";C:\Users\$env:USERNAME\AppData\Roaming\Python\Python314\Scripts;C:\arm-toolchain\bin"
$env:ZEPHYR_TOOLCHAIN_VARIANT = "gnuarmemb"
$env:GNUARMEMB_TOOLCHAIN_PATH = "C:\arm-toolchain"

# Step 2: Clear old build
Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue

# Step 3: Build new program
west build -b rpi_pico2/rp2350a/m33 programs\01_led_blink

# Step 4: Flash
# Hold BOOTSEL + plug USB → copy build\zephyr\zephyr.uf2 to RPI-RP2
```

Or use the helper script:
```powershell
.\scripts\build.ps1 01_led_blink
```

---

## Troubleshooting

| Problem | Fix |
|---------|-----|
| `west: command not found` | Add Python Scripts to PATH (see Step 2.1) |
| `arm-none-eabi-gcc not found` | Add `C:\arm-toolchain\bin` to PATH |
| `ZEPHYR_TOOLCHAIN_VARIANT not set` | Set env vars (see Step 4) |
| `No module named 'elftools'` | Run `C:\Python314\python.exe -m pip install pyelftools` |
| `RPI-RP2 drive not appearing` | Use a data USB cable, not a charge-only cable |
| LED not blinking | Check resistors (330Ω), check LED polarity (long leg = +) |
| Motor not spinning | Check ENA jumper is removed, check common GND |

---

## 🗺️ Roadmap

- [x] 01 — 3-Color LED Blink
- [x] 02 — DC Motor Control (L298N + PWM)
- [x] 03 — Dual LED Blink with RTOS Threads
- [ ] 04 — PWM LED Fade
- [ ] 05 — UART Serial Monitor
- [ ] 06 — I²C Temperature Sensor
- [ ] 07 — ADC Potentiometer
- [ ] 08 — Multi-Thread RTOS Demo
- [ ] 09 — **Edge AI Anomaly Detection** (TFLite Micro)
- [ ] 10 — Wi-Fi MQTT Sensor Dashboard

---

## 📄 License

MIT License — free to use, modify, and share.

---

## 🙋 Author

**Kshitiz Khandelwal**
Built while learning Zephyr RTOS for IoT and Edge AI projects.

> ⭐ Star this repo if it helped you get Zephyr running on your Pico 2!
