# Arduino-Sauna Controller

Sauna controller for Arduino Giga R1 WiFi + Giga Display Shield. Features temperature monitoring (DS18B20 sensors), WiFi connectivity, NTP time sync, and touchscreen UI.

## Hardware
- Arduino Giga R1 WiFi
- Giga Display Shield
- DS18B20 sensors on pin 2
- WiFi for NTP and IP info

## Setup
- Clone repo
- Add WiFi creds in src/arduino_secrets.h
- Build/upload with PlatformIO: `pio run -t upload`

## Current Version
**1.5-dev** – Phase 1 cleanup complete (Debug.h standardization, header hygiene, formatting, dead-code removal). Ready for new features.

## Roadmap
- Phase 2: Increased functionality (heater control, improved menus, etc.)
- Unit tests