# 🎮 DJI Ronin Remote Controller - ESP32/ESP8266

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![License](https://img.shields.io/badge/license-MIT-green)

A wireless remote control solution for DJI Ronin gimbals using ESP8266/ESP32 microcontrollers and WebSocket communication.

## 📝 Description

This project enables wireless control of DJI Ronin gimbals through a network connection, utilizing the SBUS protocol. It creates a bridge between a WebSocket client (like Chataigne software) and the Ronin's SBUS input, allowing remote control via gamepad or other input devices.

### ✨ Features

- 🌐 WiFi configuration portal
- 📡 WebSocket server for real-time control
- 💾 Persistent WiFi settings storage
- 📊 Real-time logging interface
- 🔧 Static IP configuration
- 🔄 Easy reset functionality
- 🎮 Compatible with Chataigne software for gamepad input

## 🛠️ Hardware Requirements

- ESP8266 or ESP32 microcontroller
- DJI Ronin gimbal (tested on Ronin SC, should work with other models)
- SBUS connection cable

## 📋 Installation Guide

### 1. Hardware Setup

1. Connect the ESP board to the Ronin's SBUS port:
   - SBUS signal → GPIO pin (as defined in code)
   - GND → GND
   - 5V → 5V (if needed)

### 2. Software Setup

1. Install required Arduino libraries:
   - ESP8266WiFi
   - ESP8266WebServer
   - DNSServer
   - EEPROM
   - WebSocketsServer
   - BMC_SBUS

2. Upload the code to your ESP device

### 3. Initial Configuration

1. Power up the device
2. Connect to the WiFi network "RoninControl_Setup"
3. Open a web browser and navigate to `192.168.4.1`
4. Configure your WiFi settings:
- Enter your network SSID and password
- Optionally configure static IP
5. Save and wait for the device to restart
  NOTE: if the ESP connects successfully to the wifi, the generated wifi will disappear.
        if the ESP can't connect to wifi for a minute, it will pass to setup mode

## 🎮 Usage

### Normal Operation

1. The device will connect to your configured WiFi network
2. Access the web interface using the device's IP address
3. Available pages:
- Status page: Shows connection info and device status
- Log page: Real-time operation logs
- WiFi Configuration: Network settings

### WebSocket Control

1. Connect to the WebSocket server on port 81
2. Send commands in format: `chX:VALUE` where:
- X: Channel number (1-16)
- VALUE: Position value (352-1696)

Example: `ch1:1024` sets channel 1 to center position

### Using with Chataigne

1. Configure Chataigne's WebSocket module to connect to the device
2. Map gamepad inputs to WebSocket messages
3. Use the following value ranges:
- Minimum: 352
- Center: 1024
- Maximum: 1696

## 🔧 Technical Details

### Key Components

- **WiFi Configuration**: Stores network settings in EEPROM
- **WebSocket Server**: Handles real-time control commands
- **SBUS Protocol**: Communicates with Ronin gimbal
- **Web Interface**: Provides configuration and monitoring

### Reset Function

- Press the RST button to clear WiFi settings
- Device will restart in setup mode
- If the device cannot connect to WiFi it will restart in setup mode

## ⚠️ Important Notes

- Tested primarily with DJI Ronin SC, but should work with other models, you will have to adjust s-bus values
- Ensure stable WiFi connection for reliable control
- Keep within range of WiFi network
- Monitor the log interface for debugging

## 🤝 Contributing

Contributions, issues, and feature requests are welcome!

## 📄 License

This project is licensed under the MIT License
