#include "backlight.h"
#include "display.h"
#include <cstring>
#include <algorithm>

// ────────────────────── グローバル変数 ──────────────────────
// 現在の輝度モード
BrightnessMode currentBrightnessMode = BrightnessMode::Day;
// ALS サンプルバッファ
uint16_t luxBuffer[MEDIAN_BUFFER_SIZE] = {};
int luxWriteIndex = 0;  // 次に書き込むインデックス


// ────────────────────── 輝度測定 ──────────────────────
// バックライトを消して輝度を測定
static uint16_t measureLuxWithoutBacklight()
{
  uint8_t prevB = display.getBrightness();
  display.setBrightness(0);
  delayMicroseconds(500);
  uint16_t lux = CoreS3.Ltr553.getAlsValue();
  display.setBrightness(prevB);
  return lux;
}

// ────────────────────── 中央値計算 ──────────────────────
// サンプル配列から中央値を計算する
static uint16_t calculateMedian(const uint16_t *samples)
{
  uint16_t sortedSamples[MEDIAN_BUFFER_SIZE];
  memcpy(sortedSamples, samples, sizeof(sortedSamples));
  std::nth_element(sortedSamples,
                   sortedSamples + MEDIAN_BUFFER_SIZE / 2,
                   sortedSamples + MEDIAN_BUFFER_SIZE);
  return sortedSamples[MEDIAN_BUFFER_SIZE / 2];
}

// ────────────────────── 輝度更新 ──────────────────────
void updateBacklightLevel()
{
  if (!SENSOR_AMBIENT_LIGHT_PRESENT) {
      if (currentBrightnessMode != BrightnessMode::Day) {
          currentBrightnessMode = BrightnessMode::Day;
          display.setBrightness(BACKLIGHT_DAY);
      }
      return;
  }

  uint16_t measuredLux = measureLuxWithoutBacklight();

  // サンプルをリングバッファへ格納
  luxBuffer[luxWriteIndex] = measuredLux;
  luxWriteIndex = (luxWriteIndex + 1) % MEDIAN_BUFFER_SIZE;

  uint16_t medianLux = calculateMedian(luxBuffer);

  BrightnessMode newMode =
      (medianLux >= LUX_THRESHOLD_DAY)  ? BrightnessMode::Day  :
      (medianLux >= LUX_THRESHOLD_DUSK) ? BrightnessMode::Dusk : BrightnessMode::Night;

  if (newMode != currentBrightnessMode) {
      currentBrightnessMode = newMode;
      uint8_t targetBrightness =
          (newMode == BrightnessMode::Day)  ? BACKLIGHT_DAY  :
          (newMode == BrightnessMode::Dusk) ? BACKLIGHT_DUSK : BACKLIGHT_NIGHT;
      display.setBrightness(targetBrightness);
  }
}

