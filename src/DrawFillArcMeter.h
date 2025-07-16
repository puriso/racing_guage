#ifndef DRAW_FILL_ARC_METER_H
#define DRAW_FILL_ARC_METER_H

#include <M5GFX.h>  // 必要なライブラリをインクルード

#include <algorithm>
#include <cmath>
#include <limits>
#include <cstring>

// std::clamp が利用できない環境向けの簡易版
template <typename T>
static inline T clampValue(T val, T low, T high) {
  if (val < low) return low;
  if (val > high) return high;
  return val;
}

// ────────────────────── 内部関数群 ──────────────────────
namespace {

// レッドゾーンの背景を描画
static void drawRedZone(M5Canvas &canvas, int centerX, int centerY,
                        int radius, int arcWidth,
                        float threshold, float minValue, float maxValue) {
  float start = -270 + ((threshold - minValue) / (maxValue - minValue) * 270.0);
  canvas.fillArc(centerX, centerY,
                 radius - arcWidth - 9,
                 radius - arcWidth - 4,
                 start, 0,
                 COLOR_RED);
}

// 目盛とラベルを描画
static void drawTicksAndLabels(M5Canvas &canvas, int centerX, int centerY,
                               int radius, int arcWidth,
                               float minValue, float maxValue,
                               float tickStep, float majorTickStep,
                               float labelStart,
                               const char *unit, const char *label) {
  const uint16_t TEXT_COLOR = COLOR_WHITE;
  const uint16_t BACKGROUND_COLOR = COLOR_BLACK;

  int tickCount = static_cast<int>((maxValue - minValue) / tickStep) + 1;
  for (float i = 0; i <= tickCount - 1; i += 1) {
    float scaled = minValue + (tickStep * i);
    float angle = 270 - ((270.0 / (tickCount - 1)) * i);
    float rad = radians(angle);

    bool isMajorTick;
    if (majorTickStep < 0) {
      isMajorTick = (fmod(scaled, 1.0f) == 0.0f);
    } else {
      float diff = fmod(scaled - labelStart, majorTickStep);
      isMajorTick = (scaled >= labelStart) &&
                    (fabsf(diff) < 0.01f || fabsf(diff - majorTickStep) < 0.01f);
    }

    int innerR = isMajorTick ? (radius - arcWidth - 10) : (radius - arcWidth - 8);
    int outerR = isMajorTick ? (radius - arcWidth - 5) : (radius - arcWidth - 7);

    int x1 = centerX + (cos(rad) * innerR);
    int y1 = centerY - (sin(rad) * innerR);
    int x2 = centerX + (cos(rad) * outerR);
    int y2 = centerY - (sin(rad) * outerR);

    canvas.drawLine(x1, y1, x2, y2, COLOR_WHITE);

    if (isMajorTick) {
      int labelX = centerX + (cos(rad) * (radius - arcWidth - 15));
      int labelY = centerY - (sin(rad) * (radius - arcWidth - 15));

      char text[6];
      snprintf(text, sizeof(text), "%.0f", scaled);

      canvas.setTextFont(1);
      canvas.setFont(&fonts::Font0);
      canvas.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
      canvas.setCursor(labelX - (canvas.textWidth(text) / 2), labelY - 4);
      canvas.print(text);
    }
  }

  char combined[30];
  snprintf(combined, sizeof(combined), "%s / %s", label, unit);
  canvas.setFont(&fonts::Font0);
  int labelX = centerX;
  int labelY = centerY + radius + 15;
  canvas.setCursor(labelX - (canvas.textWidth(combined) / 2), labelY);
  canvas.print(combined);
}

// 値を描画
static void drawValueText(M5Canvas &canvas, const char *unit, float value,
                          bool useDecimal, int valueX, int valueY) {
  const uint16_t BACKGROUND_COLOR = COLOR_BLACK;

  char valueText[10];
  char errorLine1[20];
  char errorLine2[8];
  bool isError = false;

  if (strcmp(unit, "BAR") == 0 && value >= 11.0f) {
    snprintf(errorLine1, sizeof(errorLine1), "Short circuit");
    snprintf(errorLine2, sizeof(errorLine2), "Error");
    isError = true;
  } else if (strcmp(unit, "Celsius") == 0 && value >= 199.0f) {
    snprintf(errorLine1, sizeof(errorLine1), "Disconnection");
    snprintf(errorLine2, sizeof(errorLine2), "Error");
    isError = true;
  } else if (useDecimal) {
    snprintf(valueText, sizeof(valueText), "%.1f", value);
  } else {
    snprintf(valueText, sizeof(valueText), "%.0f", round(value));
  }

  if (isError) {
    canvas.setFont(&fonts::Font0);
    int rectH = canvas.fontHeight() * 2 + 4;
    canvas.fillRect(valueX - 75, valueY - canvas.fontHeight() - 2,
                    75, rectH, BACKGROUND_COLOR);
    int line1Y = valueY - canvas.fontHeight();
    canvas.setCursor(valueX - canvas.textWidth(errorLine1), line1Y);
    canvas.print(errorLine1);
    int line2Y = line1Y + canvas.fontHeight();
    canvas.setCursor(valueX - canvas.textWidth(errorLine2), line2Y);
    canvas.print(errorLine2);
  } else {
    canvas.setFont(&FreeSansBold24pt7b);
    canvas.fillRect(valueX - 75, valueY - canvas.fontHeight() / 2 - 2,
                    75, canvas.fontHeight() + 4, BACKGROUND_COLOR);
    canvas.setCursor(valueX - canvas.textWidth(valueText),
                    valueY - (canvas.fontHeight() / 2));
    canvas.print(valueText);
  }
}

} // namespace

void drawFillArcMeter(M5Canvas &canvas, float value, float minValue, float maxValue, float threshold,
                      uint16_t overThresholdColor, const char *unit, const char *label, float &maxRecordedValue,
                      float &previousValue, // 前回描画した値
                      float tickStep,       // 目盛の間隔（細かい目盛り）
                      bool useDecimal,      // 小数点を表示するかどうか
                      int x, int y,
                      bool drawStatic,
                      float majorTickStep = -1.0f, // 数字を表示する目盛間隔（負なら旧仕様）
                      float labelStart    = 0.0f)  // ラベル描画を開始する値
{
  // 左端を 1px 固定しつつ数値表示位置は従来通りに保つ
  const int GAUGE_LEFT = x + 1;                    // 円メーターの左端
  const int CENTER_X_CORRECTED = GAUGE_LEFT + 70;  // 半径 70px を考慮した中心X座標
  const int VALUE_BASE_X = x + 160;                // 数値表示位置
  const int CENTER_Y_CORRECTED = y + 90 - 10;      // スプライト内の中心Y座標
  const int RADIUS = 70;                           // 半円メーターの半径
  const int ARC_WIDTH = 10;                        // 弧の幅

  const uint16_t ACTIVE_COLOR = COLOR_WHITE;      // 現在の値の色
  const uint16_t INACTIVE_COLOR = 0x18E3;         // メーター全体の背景色

  float clampedValue = clampValue(value, minValue, maxValue);

  // 初回描画時は背景を生成
  if (drawStatic || std::isnan(previousValue)) {
    canvas.fillArc(CENTER_X_CORRECTED, CENTER_Y_CORRECTED,
                   RADIUS - ARC_WIDTH, RADIUS,
                   -270, 0, INACTIVE_COLOR);
    previousValue = clampedValue;
  }

  if (drawStatic) {
    drawRedZone(canvas, CENTER_X_CORRECTED, CENTER_Y_CORRECTED,
                RADIUS, ARC_WIDTH, threshold, minValue, maxValue);
  }

  float prevValue = std::isnan(previousValue) ? minValue
                                             : clampValue(previousValue, minValue, maxValue);
  float prevAngle = -270 + ((prevValue - minValue) / (maxValue - minValue) * 270.0);
  float currAngle = -270 + ((clampedValue - minValue) / (maxValue - minValue) * 270.0);

  bool prevOver = prevValue >= threshold;
  bool currOver = clampedValue >= threshold;

  if (currOver) {
    if (!prevOver) {
      canvas.fillArc(CENTER_X_CORRECTED, CENTER_Y_CORRECTED,
                     RADIUS - ARC_WIDTH, RADIUS,
                     -270, currAngle, overThresholdColor);
    } else if (currAngle > prevAngle) {
      canvas.fillArc(CENTER_X_CORRECTED, CENTER_Y_CORRECTED,
                     RADIUS - ARC_WIDTH, RADIUS,
                     prevAngle, currAngle, overThresholdColor);
    } else if (currAngle < prevAngle) {
      canvas.fillArc(CENTER_X_CORRECTED, CENTER_Y_CORRECTED,
                     RADIUS - ARC_WIDTH, RADIUS,
                     currAngle, prevAngle, INACTIVE_COLOR);
    }
  } else {
    if (prevOver) {
      canvas.fillArc(CENTER_X_CORRECTED, CENTER_Y_CORRECTED,
                     RADIUS - ARC_WIDTH, RADIUS,
                     -270, currAngle, ACTIVE_COLOR);
      if (prevAngle > currAngle) {
        canvas.fillArc(CENTER_X_CORRECTED, CENTER_Y_CORRECTED,
                       RADIUS - ARC_WIDTH, RADIUS,
                       currAngle, prevAngle, INACTIVE_COLOR);
      }
    } else {
      if (currAngle > prevAngle) {
        canvas.fillArc(CENTER_X_CORRECTED, CENTER_Y_CORRECTED,
                       RADIUS - ARC_WIDTH, RADIUS,
                       prevAngle, currAngle, ACTIVE_COLOR);
      } else if (currAngle < prevAngle) {
        canvas.fillArc(CENTER_X_CORRECTED, CENTER_Y_CORRECTED,
                       RADIUS - ARC_WIDTH, RADIUS,
                       currAngle, prevAngle, INACTIVE_COLOR);
      }
    }
  }

  previousValue = clampedValue;

  if (drawStatic) {
    drawTicksAndLabels(canvas, CENTER_X_CORRECTED, CENTER_Y_CORRECTED,
                       RADIUS, ARC_WIDTH,
                       minValue, maxValue,
                       tickStep, majorTickStep,
                       labelStart, unit, label);
  }

  drawValueText(canvas, unit, value, useDecimal,
                VALUE_BASE_X, CENTER_Y_CORRECTED + RADIUS - 20);
}

#endif  // DRAW_FILL_ARC_METER_H
