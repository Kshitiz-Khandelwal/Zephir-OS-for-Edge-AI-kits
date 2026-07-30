# 02 — DC Motor Control via L298N

Demonstrates basic output control of a DC Motor using an L298N Motor Driver. The speed is controlled via PWM.

## Circuit Connections

| L298N Module | Pin | Raspberry Pi Pico |
|--------------|-----|-------------------|
| **IN1** (Direction) | - | GP2 |
| **IN2** (Direction) | - | GP3 |
| **ENA** (Speed) | - | GP4 (PWM) |
| **GND** | GND | GND (CRITICAL: Must share ground!) |

*(External power supply (6-12V) connects to the L298N's 12V and GND terminals. Motor connects to OUT1 and OUT2).*

## Pre-compiled Firmware
You can flash this program directly to your board without building it yourself.
1. Hold **BOOTSEL** on your Pico 2.
2. Plug it into your PC.
3. Drag and drop [`02_dc_motor.uf2`](02_dc_motor.uf2) into the `RPI-RP2` drive.
