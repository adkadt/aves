# Pigeon

Pigeon is a lightweight GPS and LoRa tracker built around the ESP32-S3. It is designed to provide reliable location tracking of landed model rockets.
## Features

- GPS position tracking
- LoRa communication
- Low-power friendly architecture

## Hardware

Current target platform:

- Waveshare ESP32-S3-Zero
- Adafruit Ultimate GPS
- Adafruit LoRa Radio
- WS2812 Status LED

## Project Structure

```
pigeon/
├── firmware/
│   ├── include/
│   ├── src/
│   └── platformio.ini
└── hardware/
```

## Firmware Design

The firmware is organized into independent modules.

- **LED** – Status indication and animations
- **GPS** – Position acquisition and parsing
- **LoRa** – Packet transmission and reception

Each module exposes a small public interface while hiding implementation details internally.

## Building

This project uses PlatformIO.

```bash
cd firmware
pio run
```

To upload:

```bash
pio run --target upload
```

To open the serial monitor:

```bash
pio device monitor
```

## Roadmap

- [x] Project structure
- [x] LED driver
- [ ] GPS integration
- [ ] LoRa communication
