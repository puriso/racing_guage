#include "als.h"
#include <algorithm>
#include <cstring>

uint32_t measureLuxWithoutBacklight() {
  uint8_t prevB = display.getBrightness();
  display.setBrightness(0);
  delayMicroseconds(500);
  uint32_t lux = CoreS3.Ltr553.getAlsValue();
  display.setBrightness(prevB);
  return lux;
}

void updateBacklightLevel() {
  if (!SENSOR_AMBIENT_LIGHT_PRESENT) {
    if (currentBrightnessMode != BrightnessMode::Day) {
      currentBrightnessMode = BrightnessMode::Day;
      display.setBrightness(BACKLIGHT_DAY);
    }
    return;
  }

  uint32_t lux = measureLuxWithoutBacklight();
  if (DEBUG_MODE_ENABLED) Serial.printf("[ALS] lux=%lu\n", lux);

  luxSampleBuffer[luxSampleIndex] = lux;
  luxSampleIndex = (luxSampleIndex + 1) % MEDIAN_BUFFER_SIZE;

  uint32_t sorted[MEDIAN_BUFFER_SIZE];
  memcpy(sorted, luxSampleBuffer, sizeof(sorted));
  std::nth_element(sorted, sorted + MEDIAN_BUFFER_SIZE / 2, sorted + MEDIAN_BUFFER_SIZE);
  uint32_t medianLux = sorted[MEDIAN_BUFFER_SIZE / 2];

  BrightnessMode newMode =
      (medianLux >= LUX_THRESHOLD_DAY)  ? BrightnessMode::Day  :
      (medianLux >= LUX_THRESHOLD_DUSK) ? BrightnessMode::Dusk : BrightnessMode::Night;

  if (newMode != currentBrightnessMode) {
    currentBrightnessMode = newMode;
    uint8_t targetB =
        (newMode == BrightnessMode::Day)  ? BACKLIGHT_DAY  :
        (newMode == BrightnessMode::Dusk) ? BACKLIGHT_DUSK : BACKLIGHT_NIGHT;
    display.setBrightness(targetB);

    if (DEBUG_MODE_ENABLED) {
      const char* s =
          (newMode == BrightnessMode::Day)  ? "DAY"  :
          (newMode == BrightnessMode::Dusk) ? "DUSK" : "NIGHT";
      Serial.printf("[ALS] median=%lu → mode=%s → brightness=%u\n", medianLux, s, targetB);
    }
  }
}
