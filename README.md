# 🎮 DJI Ronin Remote Controller - ESP32/ESP8266

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![License](https://img.shields.io/badge/license-MIT-green)

A wireless remote control solution for DJI Ronin gimbals using ESP8266/ESP32 microcontrollers and WebSocket communication.

## 📝 Description

This project enables wireless control of DJI Ronin gimbals through a network connection, utilizing the SBUS protocol. It creates a bridge between a WebSocket client (like Chataigne software) and the Ronin's SBUS input, allowing remote control via gamepad or other input devices.

![image](https://github.com/MrColoo/DJI-Ronin-Remote-Controller-ESP32-ESP8266/blob/main/Image%20Gallery/full-overview-1.jpg)

### ✨ Features

- 🌐 WiFi configuration portal
- 📡 WebSocket server for real-time control
- 💾 Persistent WiFi settings storage
- 📊 Real-time logging interface
- 🔧 Static IP configuration
- 🔄 Easy reset functionality
- 🎮 Compatible with Chataigne software for gamepad input or any WebSocket clients

### 🎥 Demo Video

https://github.com/user-attachments/assets/ff83e8b6-8403-40a6-982f-c90003f82979

## 🛠️ Hardware Requirements

- ESP8266 or ESP32 microcontroller
- DJI Ronin gimbal (tested on Ronin SC, should work with other models)
- Connection cables
- 1x Transistor 2N2222
- 1x 1K Resistor
- 1x 5K Resistor  
NOTE: Have a look to wiring schema image

## 📋 Installation Guide

### 1. Hardware Setup

1. Connect the ESP board to the Ronin's SBUS port:
   - SBUS signal → Transistor 2N2222 or serial inverter (look at wiring schema) -> GPIO pin (as defined in code)
   - GND → GND
   - 5V → 5V (if needed)

NOTE: Have a look to wiring schema image

![image](https://github.com/MrColoo/DJI-Ronin-Remote-Controller-ESP32-ESP8266/blob/main/DJI_SBUS_pinout.png)

![image](https://github.com/MrColoo/DJI-Ronin-Remote-Controller-ESP32-ESP8266/blob/main/RoninController-wiring_schema.png)

### 2. Software Setup

1. Install required Arduino libraries:
   - ESP8266WiFi
   - ESP8266WebServer
   - DNSServer
   - EEPROM
   - WebSocketsServer
   - Ronin_SBUS // Copy the library folder under Code folder here on github and put into your libraries folder

2. Upload the code to your ESP device

### 3. Initial Configuration

1. Power up the device
2. Connect to the WiFi network "RoninControl_Setup"
3. Automatically the captive portal should open up, if it doesn't, open a web browser and navigate to `192.168.4.1`
4. Configure your WiFi settings:
   - Enter your network SSID and password
   - Optionally configure static IP
5. Save and wait for the device to restart
  
NOTE: ✔ If the ESP connects successfully to the wifi, the generated wifi will disappear.
     ❌ If the ESP can't connect to wifi after some attempts, it will restart in setup mode

## 🎮 Usage

### Normal Operation

1. The device will connect to your configured WiFi network
2. Access the web interface using the device's IP address
3. Available pages:
- Status page: Shows connection info and device status
![image](https://github.com/MrColoo/DJI-Ronin-Remote-Controller-ESP32-ESP8266/blob/main/Image%20Gallery/Connection%20Status.png)
- Log page: Real-time operation logs
![image](https://github.com/MrColoo/DJI-Ronin-Remote-Controller-ESP32-ESP8266/blob/main/Image%20Gallery/Logs.png)
- WiFi Configuration: Network settings
![image](https://github.com/MrColoo/DJI-Ronin-Remote-Controller-ESP32-ESP8266/blob/main/Image%20Gallery/Setup%20Mode.png)

### WebSocket Control

1. Connect to the WebSocket server on port 81
2. Send commands in format: `chX:VALUE` where:
- X: Channel number (1-16)
- VALUE: Position value (352-1696)

Example: `ch1:1024` sets channel 1 to center position

### Using with Chataigne

1. Download from this repo the Chataigne Setup file
2. Configure Chataigne's WebSocket module to connect to the device
3. Map gamepad inputs to WebSocket messages
   -> Use the following value ranges:
      - Minimum: 352
      - Center: 1024
      - Maximum: 1696

NOTE: For Ronin SC I made only PAN and TILT work (channels 1 and 2)

## 🔧 Technical Details

### Key Components

- **WiFi Configuration**: Stores network settings in EEPROM
- **WebSocket Server**: Handles real-time control commands
- **SBUS Protocol**: Communicates with Ronin gimbal
- **Web Interface**: Provides configuration and monitoring

## 🐛 Known Bugs

1. The gimbal mode 2 is set by default and it is not changeable
2. If only one websocket movement command is sent (for ex. joystick stick all to the right), the gimbal will move and then stop after some seconds, instead of continuing to move.

## 🚧 Unimplemented Features (Work in progress)

1. Gimbal mode selector
2. Gimbal Recenter
3. Start and Stop Recording
4. Remote controlled motorized Zoom / Focus
5. Complete debug feature through logs

## ⚠️ Important Notes

- Tested primarily with DJI Ronin SC, but should work with other models, you may will have to adjust s-bus values in the Ronin_SBUS library
- Ensure a stable WiFi connection for reliable control
- Keep within range of the WiFi network
- Monitor the log interface for debugging

## 📚 Library

- Ronin_SBUS is a library I wrote, starting from the original [BMC_SBUS](https://github.com/boldstelvis/BMC_SBUS) library. Thanks to the developer.

## 🤝 Contributing

Contributions, issues, and feature requests are welcome!

## 📄 License

This project is licensed under the MIT License
