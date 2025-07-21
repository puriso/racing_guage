#include "backlight.h"

#include <algorithm>
#include <cstring>

#include "display.h"

// ────────────────────── グローバル変数 ──────────────────────
BrightnessMode currentBrightnessMode = BrightnessMode::Day;
// 周囲光センサーの取得値を保持するバッファ
uint16_t luxReadings[MEDIAN_BUFFER_SIZE] = {};
int luxBufferIndex = 0;

// ────────────────────── 輝度測定 ──────────────────────
// バックライトを消して輝度を測定
static uint16_t measureLuxWithoutBacklight()
{
  uint8_t prevBrightness = display.getBrightness();
  display.setBrightness(0);
  delayMicroseconds(500);
  uint16_t lux = CoreS3.Ltr553.getAlsValue();
  display.setBrightness(prevBrightness);
  return lux;
}

// ────────────────────── 輝度更新 ──────────────────────
void updateBacklightLevel()
{
  if (!SENSOR_AMBIENT_LIGHT_PRESENT)
  {
    if (currentBrightnessMode != BrightnessMode::Day)
    {
      currentBrightnessMode = BrightnessMode::Day;
      display.setBrightness(BACKLIGHT_DAY);
    }
    // センサーが無い場合はこれ以上処理しない
    return;
  }

  uint16_t lux = measureLuxWithoutBacklight();

  luxReadings[luxBufferIndex] = lux;
  luxBufferIndex = (luxBufferIndex + 1) % MEDIAN_BUFFER_SIZE;

  uint16_t sortedLux[MEDIAN_BUFFER_SIZE];
  memcpy(sortedLux, luxReadings, sizeof(sortedLux));
  std::nth_element(sortedLux, sortedLux + MEDIAN_BUFFER_SIZE / 2, sortedLux + MEDIAN_BUFFER_SIZE);
  uint16_t medianLux = sortedLux[MEDIAN_BUFFER_SIZE / 2];

  BrightnessMode newMode = (medianLux >= LUX_THRESHOLD_DAY)    ? BrightnessMode::Day
                           : (medianLux >= LUX_THRESHOLD_DUSK) ? BrightnessMode::Dusk
                                                               : BrightnessMode::Night;

  if (newMode != currentBrightnessMode)
  {
    currentBrightnessMode = newMode;
    uint8_t targetB = (newMode == BrightnessMode::Day)    ? BACKLIGHT_DAY
                      : (newMode == BrightnessMode::Dusk) ? BACKLIGHT_DUSK
                                                          : BACKLIGHT_NIGHT;
    display.setBrightness(targetB);
  }
}
