# Portable Wireless Recon & IoT Security Analyzer

## Overview

The Portable Wireless Recon & IoT Security Analyzer is an embedded cybersecurity tool built on the M5Stack Core2 (ESP32) platform. The device performs passive Wi-Fi and Bluetooth Low Energy (BLE) reconnaissance, identifies encryption methods, fingerprints nearby devices, and displays results through a touchscreen interface. Scan data can be exported over USB or Wi-Fi for further analysis.

This project was designed as a hands-on exploration of wireless security, embedded systems, and IoT threat modeling.

**⚠️ ALL FUNCTIONALITY IS PASSIVE — INTENDED FOR EDUCATIONAL PURPOSES ONLY ⚠️**

## Features

### Wi-Fi Network Scanning
- Detect nearby access points
- Identify SSID, RSSI, channel, and encryption type (Open, WPA2, WPA3)

### Bluetooth Low Energy (BLE) Scanning
- Discover nearby BLE devices
- Display MAC address, signal strength, and advertised services

### Device Fingerprinting
- Vendor identification using MAC OUI lookup
- Basic device classification (Phone, IoT Device, Access Point)
- BLE service-based device inference

### Encryption Detection
- Identify unsecured wireless networks
- Highlight weak or misconfigured IoT devices

### Touchscreen UI
- Real-time scan results
- Tab-based navigation (Wi-Fi / BLE / Devices)
- Device details view

### Data Export
- Export scan results as JSON
- USB serial output
- Optional local Wi-Fi HTTP endpoint

## Hardware Used

**M5Stack Core2**
- ESP32 dual-core MCU
- Capacitive touchscreen
- Built-in battery
- Wi-Fi + BLE radios

## Software Stack

- **Language:** C++ (Arduino framework)
- **Build System:** PlatformIO
- **IDE:** Visual Studio Code
- **Platform:** ESP32 / M5Stack Core2
- **Libraries:**
  - WiFi.h
  - ESP32 BLE Arduino
  - M5Core2
  - ArduinoJson

## System Architecture

### How It Works

The device uses the ESP32's dual-core architecture to perform concurrent Wi-Fi and BLE scanning operations. Scan results are processed, fingerprinted, and displayed on the touchscreen interface in real-time. Users can navigate between different views to examine detected networks, BLE devices, and identified device types. All scanning is performed passively without transmitting any packets or performing active reconnaissance.

## Installation & Setup

1. Install Arduino IDE or PlatformIO
2. Add ESP32 board support
3. Install required libraries:
   - M5Core2
   - ArduinoJson
   - ESP32 BLE Arduino
4. Flash the firmware to the M5Stack Core2
5. Power on and begin scanning

## Legal & Ethical Disclaimer

**This tool is intended strictly for educational purposes, defensive security research, and authorized testing environments.**

- No active attacks are performed
- No packet payloads are captured
- Users are responsible for complying with all local laws and regulations

Unauthorized scanning of networks you do not own or have permission to test may be illegal in your jurisdiction. Always obtain proper authorization before use.

## Learning Outcomes

Through this project, I gained experience with:
- Wireless security fundamentals (Wi-Fi & BLE)
- Embedded systems programming
- Concurrent task management on microcontrollers
- Hardware UI design
- Device fingerprinting techniques
- Ethical considerations in cybersecurity

## Future Improvements

- SD card logging
- Export to PC companion app
- Improved device classification
- Historical scan comparison
- WPA3 management frame detection

## Author

**Elijah Salgado**  
Computer Science — California Baptist University

---

*For questions, contributions, or feedback, please open an issue or submit a pull request.*
