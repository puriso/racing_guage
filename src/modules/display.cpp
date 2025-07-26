#include "display.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>

#include "DrawFillArcMeter.h"
#include "fps_display.h"

// ────────────────────── グローバル変数 ──────────────────────
M5GFX display;
M5Canvas mainCanvas(&display);

static bool pressureGaugeInitialized = false;
static bool waterGaugeInitialized = false;

float recordedMaxOilPressure = 0.0F;
float recordedMaxWaterTemp = 0.0F;
int recordedMaxOilTempTop = 0;

// 前回描画したゲージ値
static float prevPressureValue = std::numeric_limits<float>::quiet_NaN();
static float prevWaterTempValue = std::numeric_limits<float>::quiet_NaN();

struct DisplayCache
{
  float pressureAvg;
  float waterTempAvg;
  float oilTemp;
  int16_t maxOilTemp;
} displayCache = {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN(),
                  std::numeric_limits<float>::quiet_NaN(), INT16_MIN};

// ────────────────────── 油温バー描画 ──────────────────────
void drawOilTemperatureTopBar(M5Canvas& canvas, float oilTemp, int maxOilTemp)
{
  constexpr int MIN_TEMP = 80;
  constexpr int MAX_TEMP = 130;
  constexpr int ALERT_TEMP = 120;

  constexpr int X = 20;
  constexpr int Y = 15;
  constexpr int W = 210;
  constexpr int H = 20;
  constexpr float RANGE = MAX_TEMP - MIN_TEMP;

  canvas.fillRect(X + 1, Y + 1, W - 2, H - 2, 0x18E3);

  float drawTemp = oilTemp;
  if (drawTemp >= 199.0F)
  {
    // 異常値の場合はバーを 0 として扱う
    drawTemp = 0.0F;
  }

  if (drawTemp >= MIN_TEMP)
  {
    int barWidth = static_cast<int>(W * (drawTemp - MIN_TEMP) / RANGE);
    uint16_t barColor = (drawTemp >= ALERT_TEMP) ? COLOR_RED : COLOR_WHITE;
    canvas.fillRect(X, Y, barWidth, H, barColor);
  }

  const int marks[] = {80, 90, 100, 110, 120, 130};
  canvas.setTextSize(1);
  canvas.setTextColor(COLOR_WHITE);
  canvas.setFont(&fonts::Font0);

  for (int m : marks)
  {
    int tx = X + static_cast<int>(W * (m - MIN_TEMP) / RANGE);
    canvas.drawPixel(tx, Y - 2, COLOR_WHITE);
    canvas.setCursor(tx - 10, Y - 14);
    canvas.printf("%d", m);
    if (m == ALERT_TEMP) canvas.drawLine(tx, Y, tx, Y + H - 2, COLOR_GRAY);
  }

  canvas.setCursor(X, Y + H + 4);
  canvas.printf("OIL.T / Celsius,  MAX:%03d", maxOilTemp);
  // snprintf でバッファサイズを指定し、
  // 安全に文字列化する
  if (oilTemp >= 199.0F)
  {
    // 199℃以上は "Disconnection" と "Error" を小さなフォントで表示
    canvas.setFont(&fonts::Font0);
    canvas.drawRightString("Disconnection", LCD_WIDTH - 1, 2);
    canvas.drawRightString("Error", LCD_WIDTH - 1, 2 + canvas.fontHeight());
  }
  else
  {
    char tempStr[8];
    snprintf(tempStr, sizeof(tempStr), "%d", static_cast<int>(oilTemp));
    canvas.setFont(&FreeSansBold24pt7b);
    canvas.drawRightString(tempStr, LCD_WIDTH - 1, 2);
  }
}

// ────────────────────── 画面更新＋ログ ──────────────────────
void renderDisplayAndLog(float pressureAvg, float waterTempAvg, float oilTemp, int16_t maxOilTemp)
{
  const int TOPBAR_Y = 0;
  const int TOPBAR_H = 50;
  const int GAUGE_H = 170;

  // 水温は0.05度以上、油温は0.1度以上、油圧は0.05bar以上変化したら更新する
  bool oilChanged = std::isnan(displayCache.oilTemp) || fabs(oilTemp - displayCache.oilTemp) >= 0.1F ||
                    (maxOilTemp != displayCache.maxOilTemp);
  bool pressureChanged = std::isnan(displayCache.pressureAvg) || fabs(pressureAvg - displayCache.pressureAvg) >= 0.05F;
  bool waterChanged = std::isnan(displayCache.waterTempAvg) || fabs(waterTempAvg - displayCache.waterTempAvg) >= 0.05F;

  mainCanvas.setTextColor(COLOR_WHITE);

  if (oilChanged)
  {
    mainCanvas.fillRect(0, TOPBAR_Y, LCD_WIDTH, TOPBAR_H, COLOR_BLACK);
    maxOilTemp = std::max<float>(oilTemp, maxOilTemp);
    drawOilTemperatureTopBar(mainCanvas, oilTemp, maxOilTemp);
    displayCache.oilTemp = oilTemp;
    displayCache.maxOilTemp = maxOilTemp;
  }

  if (pressureChanged || !pressureGaugeInitialized)
  {
    if (!pressureGaugeInitialized)
    {
      mainCanvas.fillRect(0, 60, 160, GAUGE_H, COLOR_BLACK);
    }
    bool useDecimal = pressureAvg < 9.95F;
    drawFillArcMeter(mainCanvas, pressureAvg, 0.0f, MAX_OIL_PRESSURE_METER, 8.0f, COLOR_RED, "x100kPa", "OIL.P",
                     recordedMaxOilPressure, prevPressureValue, 0.5f, useDecimal, 0, 60, !pressureGaugeInitialized);
    pressureGaugeInitialized = true;
    displayCache.pressureAvg = pressureAvg;
  }

  if (waterChanged || !waterGaugeInitialized)
  {
    if (!waterGaugeInitialized)
    {
      mainCanvas.fillRect(160, 60, 160, GAUGE_H, COLOR_BLACK);
    }
    drawFillArcMeter(mainCanvas, waterTempAvg, WATER_TEMP_METER_MIN, WATER_TEMP_METER_MAX, 98.0f, COLOR_RED, "Celsius",
                     "WATER.T", recordedMaxWaterTemp, prevWaterTempValue, 1.0f, false, 160, 60, !waterGaugeInitialized,
                     5.0f, WATER_TEMP_METER_MIN);
    waterGaugeInitialized = true;
    displayCache.waterTempAvg = waterTempAvg;
  }

  bool fpsChanged = drawFpsOverlay();

  // 値が更新されたときのみスプライトを転送する
  if (oilChanged || pressureChanged || waterChanged || fpsChanged)
  {
    mainCanvas.pushSprite(0, 0);
  }
}

// ────────────────────── メーター描画更新 ──────────────────────
void updateGauges()
{
  static float smoothWaterTemp = std::numeric_limits<float>::quiet_NaN();
  static float smoothOilTemp = std::numeric_limits<float>::quiet_NaN();
  static float smoothOilPressure = std::numeric_limits<float>::quiet_NaN();

  float pressureAvg = calculateAverage(oilPressureSamples);
  pressureAvg = std::min(pressureAvg, MAX_OIL_PRESSURE_DISPLAY);
  float targetWaterTemp = calculateAverage(waterTemperatureSamples);
  float targetOilTemp = calculateAverage(oilTemperatureSamples);

  if (std::isnan(smoothWaterTemp))
  {
    smoothWaterTemp = targetWaterTemp;
  }
  if (std::isnan(smoothOilTemp))
  {
    smoothOilTemp = targetOilTemp;
  }
  if (std::isnan(smoothOilPressure))
  {
    smoothOilPressure = pressureAvg;
  }

  smoothWaterTemp += 0.1F * (targetWaterTemp - smoothWaterTemp);
  smoothOilTemp += 0.1F * (targetOilTemp - smoothOilTemp);
  smoothOilPressure += OIL_PRESSURE_SMOOTHING_ALPHA * (pressureAvg - smoothOilPressure);

  float oilTempValue = smoothOilTemp;
  float pressureValue = smoothOilPressure;
  if (!SENSOR_OIL_TEMP_PRESENT)
  {
    // センサーが無い場合は常に 0 表示
    oilTempValue = 0.0F;
  }

  recordedMaxOilPressure = std::max(recordedMaxOilPressure, pressureAvg);
  recordedMaxWaterTemp = std::max(recordedMaxWaterTemp, smoothWaterTemp);
  if (targetOilTemp < 199.0F)
  {
    recordedMaxOilTempTop = std::max(recordedMaxOilTempTop, static_cast<int>(targetOilTemp));
  }

  renderDisplayAndLog(pressureValue, smoothWaterTemp, oilTempValue, recordedMaxOilTempTop);
}
