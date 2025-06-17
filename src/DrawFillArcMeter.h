#ifndef DRAW_FILL_ARC_METER_H
#define DRAW_FILL_ARC_METER_H

#include <M5GFX.h>  // 必要なライブラリをインクルード
#include <algorithm>
#include <cmath>

// 描画状態保持用構造体
struct GaugeRenderState {
  bool  firstDraw      = true;                                  // 初回描画かどうか
  float previousValue  = std::numeric_limits<float>::quiet_NaN();
  int   previousDigits = 0;                                     // 前回描画時の桁数
};

void drawFillArcMeter(M5Canvas &canvas, float value, float minValue, float maxValue, float threshold,
                      uint16_t overThresholdColor, const char *unit, const char *label, float &maxRecordedValue,
                      float tickStep,  // 目盛の間隔
                      bool useDecimal,  // 小数点を表示するかどうか
                      int x, int y,
                      GaugeRenderState &state
)
{
  const int CENTER_X_CORRECTED = x + 75 + 5;   // スプライト内の中心X座標
  const int CENTER_Y_CORRECTED = y + 90 - 10;  // スプライト内の中心Y座標
  const int RADIUS = 70;                   // 半円メーターの半径
  const int ARC_WIDTH = 10;                // 弧の幅

  const uint16_t BACKGROUND_COLOR = BLACK;                // 背景色
  const uint16_t ACTIVE_COLOR = WHITE;                    // 現在の値の色
  const uint16_t INACTIVE_COLOR = 0x18E3;                 // メーター全体の背景色
  const uint16_t TEXT_COLOR = WHITE;                      // テキストの色

  // 最大値を更新
  maxRecordedValue = std::max(value, maxRecordedValue);

  // 毎回バー部分のみ背景色でクリア
  canvas.fillArc(CENTER_X_CORRECTED, CENTER_Y_CORRECTED,
                 RADIUS - ARC_WIDTH, RADIUS,
                 -270, 0, INACTIVE_COLOR);

  // 初回のみレッドゾーンと目盛りを描画
  if (state.firstDraw) {
    float redZoneStartAngle =
        -270 + ((threshold - minValue) / (maxValue - minValue) * 270.0);
    canvas.fillArc(CENTER_X_CORRECTED, CENTER_Y_CORRECTED,
                   RADIUS - ARC_WIDTH - 9,  // 内側半径
                   RADIUS - ARC_WIDTH - 4,  // 外側半径
                   redZoneStartAngle, 0,
                   RED);               // レッドゾーンは常に赤表示

    int tickCount = static_cast<int>((maxValue - minValue) / tickStep) + 1;
    for (float i = 0; i <= tickCount - 1; i += 1)
    {
      float scaledValue = minValue + (tickStep * i);
      float angle = 270 - ((270.0 / (tickCount - 1)) * i);  // 開始位置のロジックを維持
      float rad = radians(angle);

      int lineX1 = CENTER_X_CORRECTED + (cos(rad) * (RADIUS - ARC_WIDTH - 10));
      int lineY1 = CENTER_Y_CORRECTED - (sin(rad) * (RADIUS - ARC_WIDTH - 10));
      int lineX2 = CENTER_X_CORRECTED + (cos(rad) * (RADIUS - ARC_WIDTH - 5));
      int lineY2 = CENTER_Y_CORRECTED - (sin(rad) * (RADIUS - ARC_WIDTH - 5));

      canvas.drawLine(lineX1, lineY1, lineX2, lineY2, WHITE);

      // 整数値の目盛ラベルを描画
      if (fmod(scaledValue, 1.0) == 0)
      {
        int labelX = CENTER_X_CORRECTED + (cos(rad) * (RADIUS - ARC_WIDTH - 15));
        int labelY = CENTER_Y_CORRECTED - (sin(rad) * (RADIUS - ARC_WIDTH - 15));

        char labelText[6];
        snprintf(labelText, sizeof(labelText), "%.0f", scaledValue);

        canvas.setTextFont(1);
        canvas.setFont(&fonts::Font0);
        canvas.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
        canvas.setCursor(labelX - (canvas.textWidth(labelText) / 2), labelY - 4);
        canvas.print(labelText);
      }
    }
  }

  // 現在の値に対応する部分を塗りつぶし
  if (value >= minValue && value <= maxValue * 1.1)
  {
    uint16_t barColor = (value >= threshold) ? overThresholdColor : ACTIVE_COLOR;
    float valueAngle = -270 + ((value - minValue) / (maxValue - minValue) * 270.0);
    canvas.fillArc(CENTER_X_CORRECTED, CENTER_Y_CORRECTED, RADIUS - ARC_WIDTH, RADIUS, -270, valueAngle, barColor);
  }



  // 値を右下に表示
  char valueText[10];
  if (useDecimal)
  {
    snprintf(valueText, sizeof(valueText), "%.1f", value);
  }
  else
  {
    snprintf(valueText, sizeof(valueText), "%.0f", round(value));
  }

  canvas.setFont(&FreeSansBold24pt7b);
  int valueX = CENTER_X_CORRECTED + RADIUS + 10;
  int valueY = CENTER_Y_CORRECTED + RADIUS - 20;

  // 表示領域を一度クリア
  char maxStr[6];
  if (useDecimal)
    strcpy(maxStr, "00.0");
  else
    strcpy(maxStr, "000");
  int clearW = canvas.textWidth(maxStr) + 4;
  int clearH = canvas.fontHeight();
  canvas.fillRect(valueX - clearW, valueY - clearH, clearW, clearH, BACKGROUND_COLOR);

  canvas.setTextColor(TEXT_COLOR);
  canvas.setCursor(valueX - canvas.textWidth(valueText), valueY - (canvas.fontHeight() / 2));
  canvas.print(valueText);

  // 単位とメーター名を表示
  char combinedLabel[30];
  snprintf(combinedLabel, sizeof(combinedLabel), "%s / %s", label, unit);
  canvas.setFont(&fonts::Font0);
  int labelX = CENTER_X_CORRECTED;
  int labelY = CENTER_Y_CORRECTED + RADIUS + 15;
  canvas.setCursor(labelX - (canvas.textWidth(combinedLabel) / 2), labelY);
  canvas.print(combinedLabel);

  // 状態を更新
  state.firstDraw = false;
  state.previousValue = value;
  if (!useDecimal) {
    state.previousDigits = (static_cast<int>(value) >= 100) ? 3 : 2;
  }
}

#endif // DRAW_FILL_ARC_METER_H
