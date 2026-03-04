# Arduino-Sauna Full Specification

## Overview
[High-level goal — one paragraph]

## Hardware
- Board: Arduino Giga R1 WiFi
- Display: Giga Display Shield
- Sensors: 2× DS18B20 (sauna air + heater area)
- Heater control: [relay pin TBD]
- WiFi + NTP

## Core Features (Current + Phase 2)
- Temperature monitoring & display
- WiFi + time sync
- Touchscreen UI (menus, setpoints)
- Heater control with PID/hysteresis (Phase 2)
- Safety state machine (Phase 2)
- [Future: app control, logging, etc.]

## Safety Requirements
- List every hard rule here (copy from PROJECT_RULES)

## Non-Functional
- Update rate: 1–2 seconds
- No blocking code
- Reliable after power loss
- Memory usage < 50% on Giga

## Roadmap
- Phase 2: Heater + full state machine
- Phase 3: ...