#include "display.h"
#include "DrawFillArcMeter.h"
#include <algorithm>
#include <cmath>
#include <limits>

// ────────────────────── グローバル変数 ──────────────────────
M5GFX display;
M5Canvas mainCanvas(&display);

static bool pressureGaugeInitialized = false;
static bool waterGaugeInitialized = false;

float recordedMaxOilPressure = 0.0f;
float recordedMaxWaterTemp = 0.0f;
int recordedMaxOilTempTop = 0;

struct DisplayCache {
    float pressureAvg;
    float waterTempAvg;
    float oilTemp;
    int16_t maxOilTemp;
} displayCache = {std::numeric_limits<float>::quiet_NaN(),
                  std::numeric_limits<float>::quiet_NaN(),
                  std::numeric_limits<float>::quiet_NaN(),
                  INT16_MIN};

// ────────────────────── 油温バー描画 ──────────────────────
void drawOilTemperatureTopBar(M5Canvas& canvas, float oilTemp, int maxOilTemp)
{
    constexpr int MIN_TEMP   =  80;
    constexpr int MAX_TEMP   = 130;
    constexpr int ALERT_TEMP = 120;

    constexpr int X = 20, Y = 15, W = 210, H = 20;
    constexpr float RANGE = MAX_TEMP - MIN_TEMP;

    canvas.fillRect(X + 1, Y + 1, W - 2, H - 2, 0x18E3);

    // 異常値の場合は Err 表示のみ
    if (std::isnan(oilTemp)) {
        canvas.setFont(&FreeSansBold24pt7b);
        canvas.drawRightString("Err", LCD_WIDTH - 1, 2);
        return;
    }

    if (oilTemp >= MIN_TEMP) {
        int barWidth = static_cast<int>(W * (oilTemp - MIN_TEMP) / RANGE);
        uint32_t barColor = (oilTemp >= ALERT_TEMP) ? COLOR_RED : COLOR_WHITE;
        canvas.fillRect(X, Y, barWidth, H, barColor);
    }

    const int marks[] = {80, 90, 100, 110, 120, 130};
    canvas.setTextSize(1);
    canvas.setTextColor(COLOR_WHITE);
    canvas.setFont(&fonts::Font0);

    for (int m : marks) {
        int tx = X + static_cast<int>(W * (m - MIN_TEMP) / RANGE);
        canvas.drawPixel(tx, Y - 2, COLOR_WHITE);
        canvas.setCursor(tx - 10, Y - 14);
        canvas.printf("%d", m);
        if (m == ALERT_TEMP)
            canvas.drawLine(tx, Y, tx, Y + H - 2, COLOR_GRAY);
    }

    canvas.setCursor(X, Y + H + 4);
    canvas.printf("OIL.T / Celsius,  MAX:%03d", maxOilTemp);
    char tempStr[6];
    sprintf(tempStr, "%d", static_cast<int>(oilTemp));
    canvas.setFont(&FreeSansBold24pt7b);
    canvas.drawRightString(tempStr, LCD_WIDTH - 1, 2);
}

// ────────────────────── 画面更新＋ログ ──────────────────────
void renderDisplayAndLog(float pressureAvg, float waterTempAvg,
                         float oilTemp, int16_t maxOilTemp)
{
    const int TOPBAR_Y = 0, TOPBAR_H = 50;
    const int GAUGE_H  = 170;

    bool oilChanged = std::isnan(displayCache.oilTemp) || std::isnan(oilTemp) ||
                      fabs(oilTemp - displayCache.oilTemp) > 0.01f ||
                      (maxOilTemp != displayCache.maxOilTemp);
    bool pressureChanged = std::isnan(displayCache.pressureAvg) || std::isnan(pressureAvg) ||
                           fabs(pressureAvg - displayCache.pressureAvg) > 0.01f;
    bool waterChanged    = std::isnan(displayCache.waterTempAvg) || std::isnan(waterTempAvg) ||
                           fabs(waterTempAvg - displayCache.waterTempAvg) > 0.01f;

    mainCanvas.setTextColor(COLOR_WHITE);

    if (oilChanged) {
        mainCanvas.fillRect(0, TOPBAR_Y, LCD_WIDTH, TOPBAR_H, COLOR_BLACK);
        if (oilTemp > maxOilTemp) maxOilTemp = oilTemp;
        drawOilTemperatureTopBar(mainCanvas, oilTemp, maxOilTemp);
        displayCache.oilTemp    = oilTemp;
        displayCache.maxOilTemp = maxOilTemp;
    }

    if (pressureChanged || !pressureGaugeInitialized) {
        if (!pressureGaugeInitialized) {
            mainCanvas.fillRect(0, 60, 160, GAUGE_H, COLOR_BLACK);
        }
        bool useDecimal = pressureAvg < 9.95f;
        drawFillArcMeter(mainCanvas, pressureAvg,  0.0f, MAX_OIL_PRESSURE_METER,  8.0f,
                         COLOR_RED, "BAR", "OIL.P", recordedMaxOilPressure,
                         0.5f, useDecimal,   0,   60,
                         !pressureGaugeInitialized);
        pressureGaugeInitialized = true;
        displayCache.pressureAvg = pressureAvg;
    }

    if (waterChanged || !waterGaugeInitialized) {
        if (!waterGaugeInitialized) {
            mainCanvas.fillRect(160, 60, 160, GAUGE_H, COLOR_BLACK);
        }
        drawFillArcMeter(mainCanvas, waterTempAvg, WATER_TEMP_METER_MIN, WATER_TEMP_METER_MAX, 98.0f,
                         COLOR_RED, "Celsius", "WATER.T", recordedMaxWaterTemp,
                         1.0f, false, 160,  60,
                         !waterGaugeInitialized,
                         5.0f, WATER_TEMP_METER_MIN);
        waterGaugeInitialized = true;
        displayCache.waterTempAvg = waterTempAvg;
    }

    if (DEBUG_MODE_ENABLED) {
        mainCanvas.fillRect(0, LCD_HEIGHT - 16, 80, 16, COLOR_BLACK);
        mainCanvas.setFont(&fonts::Font0);
        mainCanvas.setTextSize(0);
        mainCanvas.setCursor(5, LCD_HEIGHT - 12);
        mainCanvas.printf("FPS:%d", currentFramesPerSecond);
    }

    mainCanvas.pushSprite(0, 0);
}

// ────────────────────── メーター描画更新 ──────────────────────
void updateGauges()
{
    static float smoothWaterTemp = std::numeric_limits<float>::quiet_NaN();
    static float smoothOilTemp   = std::numeric_limits<float>::quiet_NaN();

    float pressureAvg     = calculateAverage(oilPressureSamples);
    pressureAvg = std::min(pressureAvg, MAX_OIL_PRESSURE_DISPLAY);
    bool pressureErr      = isErratic(oilPressureSamples, PRESSURE_ERR_THRESHOLD);

    float targetWaterTemp = calculateAverage(waterTemperatureSamples);
    bool waterErr         = isErratic(waterTemperatureSamples, TEMP_ERR_THRESHOLD);

    float targetOilTemp   = calculateAverage(oilTemperatureSamples);
    bool oilErr           = isErratic(oilTemperatureSamples, TEMP_ERR_THRESHOLD);

    if (std::isnan(smoothWaterTemp)) smoothWaterTemp = targetWaterTemp;
    if (std::isnan(smoothOilTemp))   smoothOilTemp   = targetOilTemp;

    smoothWaterTemp += 0.1f * (targetWaterTemp - smoothWaterTemp);
    smoothOilTemp   += 0.1f * (targetOilTemp   - smoothOilTemp);

    float oilTempValue = smoothOilTemp;
    if (oilErr) oilTempValue = std::numeric_limits<float>::quiet_NaN();
    if (pressureErr) pressureAvg = std::numeric_limits<float>::quiet_NaN();
    if (waterErr) smoothWaterTemp = std::numeric_limits<float>::quiet_NaN();

    if (!SENSOR_OIL_TEMP_PRESENT) oilTempValue = 0.0f;

    if (!pressureErr)
        recordedMaxOilPressure = std::max(recordedMaxOilPressure, pressureAvg);
    if (!waterErr)
        recordedMaxWaterTemp   = std::max(recordedMaxWaterTemp, smoothWaterTemp);
    if (!oilErr)
        recordedMaxOilTempTop = std::max(recordedMaxOilTempTop, static_cast<int>(targetOilTemp));

    renderDisplayAndLog(pressureAvg, smoothWaterTemp,
                        oilTempValue, recordedMaxOilTempTop);
}
