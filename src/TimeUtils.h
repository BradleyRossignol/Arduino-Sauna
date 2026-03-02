#pragma once

#ifndef TIME_UTILS_H
#define TIME_UTILS_H

// ── File Overview ──────────────────────────────────────────────────────────
// Lightweight epoch (Unix timestamp) to broken-down date/time converter.
// Used by Network Info screen and any RTC/NTP features.
// No dependencies beyond Arduino core. Pure calculation – no external libraries.
// ───────────────────────────────────────────────────────────────────────────

#include <Arduino.h>

void epochToDateTime(unsigned long epoch,
                     int& year, int& month, int& day,
                     int& hour, int& minute, int& second);

#endif