# Talon HMI Glove

![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)
![Platform](https://img.shields.io/badge/platform-ESP32-lightgrey.svg)
![OS](https://img.shields.io/badge/OS-Windows-green.svg)

**Talon** is a custom 13-Degree-of-Freedom (DoF) wearable Human-Machine Interface (HMI) designed for zero-latency 3D CAD manipulation, spatial computing, and tactile hardware interaction. 

It fuses an ESP-WROOM-32 microcontroller with a custom analog sensor array and a native Windows C++ driver to seamlessly map physical hand movements into operating system-level cursor control and macro inputs.

---

## ⚙️ System Architecture

### 1. Hardware & Sensors
Talon utilizes a 74HC4067 16-Channel Analog Multiplexer to rapidly poll a localized sensor network, feeding data into the ESP32 via a single ADC1 pin.

* **Spatial Tracking (IMU):** MPU-6050 6-Axis Accelerometer and Gyroscope for wrist pitch and yaw (mapped to relative X/Y screen coordinates).
* **Kinematic Tracking:** * 3x Flex Sensors encased in dorsal straps (Thumb, Index, Middle).
  * 6x Trimmer Potentiometers mounted at the proximal phalange joints.
* **Tactile Input:** 3x FSR (Force Sensing Resistor) pressure pads located on the ventral fingertips to register pinch forces and left-clicks.
* **Haptic & Audio Feedback:** A 12mm Piezoelectric disc (brass/ceramic with aluminum shell) mounted to the index fingertip. Driven by an NPN Transistor (2N2222) PWM circuit, it acts as both a sharp haptic buzzer and a high-frequency bone-conductive audio transducer.

### 2. Power Management (Untethered Ready)
* **Battery:** 3.7V 2600mAh Li-ion pouch cell (505573).
* **Load-Sharing & Charging:** Integrated TP4056 Type-C module combined with a P-Channel MOSFET and Schottky diode power-path circuit. This allows simultaneous battery charging and ESP32 operation via a single USB-C cable without back-powering the lithium cell.

### 3. Mechanical Design
* **Finger Sleeves:** 3D Printed in flexible TPU (15% Gyroid infill). Printed horizontally to maximize Z-axis tensile strength against continuous bending. Wires are routed using support-free C-channel snap slots.
* **Central Hub:** 3D Printed in rigid ABS for high dielectric insulation and thermal stability.

---

## 💻 Software & Firmware

### ESP32 Core Firmware (`/firmware`)
Written in C++ (PlatformIO), the firmware operates as a high-speed data aggregator. It polls the MPU-6050 via I2C and cycles the 74HC4067 multiplexer to read all 12 analog sensors in under 1.5 milliseconds. Data is serialized and blasted over a tethered UART connection at **115200 baud** for an ultra-fluid 50Hz+ refresh rate.

### Windows OS Driver (`/software/pc_client`)
A lightweight, background C++ executable that interfaces directly with the `<windows.h>` API. 
* Parses the incoming CSV serial stream.
* Maps IMU telemetry to dynamic `SetCursorPos()` movements using deadzone logic.
* Maps FSR threshold values to `SendInput()` hardware-level mouse clicks.
* Transmits reverse serial commands to trigger the ESP32's haptic piezo circuit upon successful interaction.

---

## ⚠️ Engineering Notes & Hardware Constraints

**Why Tethered Serial over Bluetooth/BLE?**
The standard ESP32 silicon shares its ADC2 module directly with the 2.4GHz radio transceiver. Activating Bluetooth Classic or BLE instantly disables all ADC2 pins (`analogRead` fails). 

To read 12 simultaneous analog sensors without relying exclusively on external multiplexers for every pin, this V1 iteration runs strictly tethered over USB, completely bypassing the radio/ADC2 hardware conflict while guaranteeing zero-latency CAD manipulation. The addition of the 74HC4067 multiplexer strictly routes signals to ADC1, future-proofing the board for a wireless V2 upgrade.

---

## 🛠️ Getting Started

### Prerequisites
* [VS Code](https://code.visualstudio.com/) with the [PlatformIO Extension](https://platformio.org/) (for firmware).
* A C++ Compiler (e.g., MinGW or MSVC) for the Windows driver.

### Installation
1. **Clone the repo:**
   ```bash
   git clone [https://github.com/YourUsername/Talon-HMI-Glove.git](https://github.com/YourUsername/Talon-HMI-Glove.git)
