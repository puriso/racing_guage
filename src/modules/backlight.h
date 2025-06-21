#ifndef BACKLIGHT_H
#define BACKLIGHT_H

#include "config.h"

extern BrightnessMode currentBrightnessMode;

constexpr uint32_t ALS_MEASUREMENT_INTERVAL_MS = 8000;  // ALS 測定間隔 [ms]

void updateBacklightLevel();

#endif // BACKLIGHT_H
