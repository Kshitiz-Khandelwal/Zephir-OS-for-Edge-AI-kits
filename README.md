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

Each program has its own folder containing the source code, pre-compiled `.uf2` file, and detailed wiring instructions.

### 01 — 3-Color LED Blink
**Folder:** `programs/01_led_blink/`
- Sequentially blinks 3 external LEDs.
- [View Wiring & Details](programs/01_led_blink/)

### 02 — DC Motor Control (L298N)
**Folder:** `programs/02_dc_motor/`
- Controls a DC Motor using an L298N driver and PWM for speed control.
- [View Wiring & Details](programs/02_dc_motor/)

### 03 — Dual LED Blink with RTOS Threads
**Folder:** `programs/03_threads_dual_led/`
- Uses `K_THREAD_DEFINE` to blink two LEDs at different rates simultaneously.
- [View Wiring & Details](programs/03_threads_dual_led/)

### 04 — Emergency Monitor & Non-Blocking Timers
**Folder:** `programs/04_emergency_monitor/`
- Non-blocking main loop timers plus a dedicated thread monitoring a sensor.
- [View Wiring & Details](programs/04_emergency_monitor/)

### 05 — TM1637 Display & Heartbeat (Bit-Banging)
**Folder:** `programs/05_tm1637_display/`
- Custom Bit-Banged communication protocol in C with concurrent LED heartbeat.
- [View Wiring & Concepts Explained](programs/05_tm1637_display/)

### 06 — Thread Priorities & Preemption
**Folder:** `programs/06_thread_priorities/`
- Spawns 3 independent RTOS threads and uses thread priority to allow an emergency task to preempt normal tasks.
- [View Wiring & Concepts Explained](programs/06_thread_priorities/)

### 07 — Semaphores & Cross-Thread Signaling
**Folder:** `programs/07_semaphores/`
- Uses an RTOS Semaphore to signal a Display Thread to update the TM1637 (OPEN/LOCK) when an Input Thread detects a button press.
- [View Wiring & Concepts Explained](programs/07_semaphores/)

### 08 — Mutexes & Resource Protection
**Folder:** `programs/08_mutex/`
- Uses an RTOS Mutex to protect a shared resource (the TM1637 display) from being simultaneously accessed and corrupted by two different threads.
- [View Wiring & Concepts Explained](programs/08_mutex/)

### 09 — Message Queues (Data Passing)
**Folder:** `programs/09_message_queue/`
- Uses an RTOS Message Queue to safely pass data (simulated temperature) between a Producer Thread and a Consumer Display Thread without using global variables.
- [View Wiring & Concepts Explained](programs/09_message_queue/)

### 10 — Software Timers (No Delays)
**Folder:** `programs/10_software_timers/`
- Uses an RTOS Kernel Software Timer (`k_timer`) to trigger an LED blink precisely every 5 seconds, entirely eliminating the need for threads or blocking delay loops.
- [View Wiring & Concepts Explained](programs/10_software_timers/)

### 11 — GPIO Interrupts (Hardware ISR)
**Folder:** `programs/11_gpio_interrupt/`
- Configures a hardware interrupt on GP2 so the CPU instantly halts whatever it's doing (even sleeping) to service a button press with zero latency, entirely replacing inefficient polling.
- [View Wiring & Concepts Explained](programs/11_gpio_interrupt/)

### 12 — PWM Task (Concurrent Fading)
**Folder:** `programs/12_pwm_task/`
- Demonstrates RTOS preemption by running a rapid 500Hz Software PWM loop to fade an RGB LED, while seamlessly allowing a background Status Thread to wake up and print exactly on time without jitter.
- [View Wiring & Concepts Explained](programs/12_pwm_task/)

---

## Concepts & Theory 🧠
If you're learning about Embedded Systems alongside these programs, check out the detailed documentation here:
- [Sleep vs. Delay vs. Blocking in Embedded Systems](docs/Sleep_vs_Delay.md)

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
