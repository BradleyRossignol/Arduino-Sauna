# Arduino-Sauna PROJECT RULES (Grok Edition) - v1.0
# Always paste relevant sections when asking Grok for code

## Project Context
- Safety-critical sauna controller
- Hardware: Arduino Giga R1 WiFi (M7) + Giga Display Shield + 2× DS18B20 (OneWire pin 2)
- Build system: PlatformIO (giga_r1_m7)
- Current state: 1.5-dev (Phase 1 cleanup complete)

## Safety Rules (NON-NEGOTIABLE)
- Heater MUST default to OFF on boot and after any error
- Immediate heater shutdown if ANY sensor > 115°C or fails
- Hardcoded MAX_SAFE_TEMP = 115, MAX_HEATER_TIME_MINUTES = 45
- Fail-safe relay logic (active HIGH or LOW — confirm pin later)
- Every safety event must be logged via Debug.h

## Architecture Rules
- Continue Manager pattern (SensorManager, WiFiManager, DisplayManager, HeaterManager, etc.)
- Central Finite State Machine in main.ino or SaunaController (states: Off, Preheating, Heating, Maintaining, Cooldown, Error)
- UI layer must remain non-blocking
- All config in Config.h or arduino_secrets.h
- New features = new Manager.h/.cpp in src/

## Coding Standards
- Modern C++ (C++17 where Arduino allows)
- Naming: camelCase variables/methods, PascalCase classes
- Prefer static memory, avoid dynamic allocation and String abuse
- Use millis() timing only — no delay() in loop()
- Every public method and state transition gets a clear comment
- Keep functions < 60 lines when possible
- Follow existing Debug.h style for logging

## File Organization (do not change existing)
- src/ for .cpp
- include/ for .h
- Keep main.ino minimal (setup/loop + state machine + manager calls)
- Tests go in test/

## Grok Interaction Rules
- Always output ONLY changed files in diff format when asked
- Never remove existing safety code
- Suggest unit tests for new managers
- Ask for pin confirmation before finalizing hardware code