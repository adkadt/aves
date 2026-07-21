<div align="center">

# AVES
### Aerospace Vehicle Embedded Suite

*A personal collection of open-source avionics projects for model rocketry.*

</div>

---

## Overview

AVES is a collection of hardware and software projects developed for model rockets. The goal is to design reliable, flight-ready avionics while documenting the engineering process along the way.

Each project is developed independently and focuses on solving a specific problem, such as flight computers, telemetry, or tracking.

While AVES is currently a personal project, all repositories are open source and may serve as useful references for others interested in rocketry and embedded systems.

---

## Projects

| Project | Description | Status |
|---------|-------------|--------|
| **Pigeon** | GPS and LoRa tracker for rocket recovery | In Development |
| **Shrike** | Dual-deployment flight computer and altimeter | Early Development |
| **Wren** | Dual-deployment flight computer with integrated IMU and altimeter | Early Development |
| **GPS Ground Station** | Microcontroller-based LoRa receiver for displaying GPS telemetry | In Development |

---

## Goals

- Build reliable avionics for model rockets
- Learn through hands-on embedded development
- Produce well-documented hardware and firmware
- Keep projects maintainable and easy to understand

---

## Technologies

Projects within AVES make use of technologies such as:

- C++
- PlatformIO
- ESP32
- GPS
- LoRa
- Barometric sensors
- IMUs

The exact hardware varies by project.

---

## Repository Layout

```
AVES/
├── gps-ground-station/
├── pigeon/
├── shrike/
├── wren/
└── README.md
```

Each project contains its own firmware and documentation.
