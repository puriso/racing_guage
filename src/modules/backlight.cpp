#include "backlight.h"
#include "display.h"
#include <cstring>
#include <algorithm>
#include <cmath>

// ────────────────────── グローバル変数 ──────────────────────
BrightnessMode currentBrightnessMode = BrightnessMode::Day;
uint16_t luxSampleBuffer[MEDIAN_BUFFER_SIZE] = {};
int luxSampleIndex = 0;
static float smoothBrightness = BACKLIGHT_DAY;
static uint8_t targetBrightness = BACKLIGHT_DAY;


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

// ────────────────────── 輝度更新 ──────────────────────
void updateBacklightLevel()
{
    if (!SENSOR_AMBIENT_LIGHT_PRESENT) {
        // ALS 無効時は常に昼モード
        currentBrightnessMode = BrightnessMode::Day;
        targetBrightness      = BACKLIGHT_DAY;
    } else {
        uint16_t lux = measureLuxWithoutBacklight();

        luxSampleBuffer[luxSampleIndex] = lux;
        luxSampleIndex = (luxSampleIndex + 1) % MEDIAN_BUFFER_SIZE;

        uint16_t sorted[MEDIAN_BUFFER_SIZE];
        memcpy(sorted, luxSampleBuffer, sizeof(sorted));
        std::nth_element(sorted, sorted + MEDIAN_BUFFER_SIZE / 2,
                        sorted + MEDIAN_BUFFER_SIZE);
        uint16_t medianLux = sorted[MEDIAN_BUFFER_SIZE / 2];

        BrightnessMode newMode =
            (medianLux >= LUX_THRESHOLD_DAY)  ? BrightnessMode::Day  :
            (medianLux >= LUX_THRESHOLD_DUSK) ? BrightnessMode::Dusk : BrightnessMode::Night;

        if (newMode != currentBrightnessMode) {
            currentBrightnessMode = newMode;
            targetBrightness =
                (newMode == BrightnessMode::Day)  ? BACKLIGHT_DAY  :
                (newMode == BrightnessMode::Dusk) ? BACKLIGHT_DUSK : BACKLIGHT_NIGHT;
        }
    }

    // 目標輝度へ徐々に近づける
    float diff = static_cast<float>(targetBrightness) - smoothBrightness;
    float step = diff * BACKLIGHT_FILTER_ALPHA;

    if (step > BACKLIGHT_TRANSITION_SPEED)
        step = BACKLIGHT_TRANSITION_SPEED;
    else if (step < -BACKLIGHT_TRANSITION_SPEED)
        step = -BACKLIGHT_TRANSITION_SPEED;

    if (std::fabs(diff) <= 0.5f) {
        smoothBrightness = targetBrightness;
    } else {
        smoothBrightness += step;
    }

    display.setBrightness(static_cast<uint8_t>(smoothBrightness + 0.5f));
}

