#include "DrawFillArcMeter.h"
#include <M5GFX.h>
#include <algorithm>
#include <cmath>

void drawFillArcMeter(M5Canvas& canvas, float value, float minValue, float maxValue,
                      float threshold, uint16_t overThresholdColor, const char* unit,
                      const char* label, float& maxRecordedValue, float tickStep,
                      bool useDecimal, int x, int y, bool drawStatic,
                      float majorTickStep, float labelStart) {
  const int GAUGE_LEFT = x + 1;
  const int CENTER_X_CORRECTED = GAUGE_LEFT + 70;
  const int VALUE_BASE_X = x + 160;
  const int CENTER_Y_CORRECTED = y + 90 - 10;
  const int RADIUS = 70;
  const int ARC_WIDTH = 10;

  const uint16_t BACKGROUND_COLOR = COLOR_BLACK;
  const uint16_t ACTIVE_COLOR = COLOR_WHITE;
  const uint16_t INACTIVE_COLOR = 0x18E3;
  const uint16_t TEXT_COLOR = COLOR_WHITE;
  (void)TEXT_COLOR; // 互換のため

  float clampedValue = value;
  if (clampedValue < minValue)
    clampedValue = minValue;
  else if (clampedValue > maxValue)
    clampedValue = maxValue;
  maxRecordedValue = std::max(clampedValue, maxRecordedValue);

  canvas.fillArc(CENTER_X_CORRECTED, CENTER_Y_CORRECTED,
                 RADIUS - ARC_WIDTH, RADIUS, -270, 0, INACTIVE_COLOR);

  if (drawStatic) {
    float redZoneStartAngle = -270 + ((threshold - minValue) / (maxValue - minValue) * 270.0);
    canvas.fillArc(CENTER_X_CORRECTED, CENTER_Y_CORRECTED,
                   RADIUS - ARC_WIDTH - 9,
                   RADIUS - ARC_WIDTH - 4,
                   redZoneStartAngle, 0,
                   COLOR_RED);
  }

  if (clampedValue >= minValue) {
    uint16_t barColor = (value >= threshold) ? overThresholdColor : ACTIVE_COLOR;
    float valueAngle = -270 + ((clampedValue - minValue) / (maxValue - minValue) * 270.0);
    canvas.fillArc(CENTER_X_CORRECTED, CENTER_Y_CORRECTED,
                   RADIUS - ARC_WIDTH, RADIUS, -270, valueAngle, barColor);
  }

  if (drawStatic) {
    int tickCount = static_cast<int>((maxValue - minValue) / tickStep) + 1;
    for (float i = 0; i <= tickCount - 1; i += 1) {
      float scaledValue = minValue + (tickStep * i);
      float angle = 270 - ((270.0 / (tickCount - 1)) * i);
      float rad = radians(angle);

      bool isMajorTick;
      if (majorTickStep < 0) {
        isMajorTick = (fmod(scaledValue, 1.0f) == 0.0f);
      } else {
        float diff = fmod(scaledValue - labelStart, majorTickStep);
        isMajorTick = (scaledValue >= labelStart) &&
                      (fabsf(diff) < 0.01f || fabsf(diff - majorTickStep) < 0.01f);
      }

      int innerRadius = isMajorTick ? (RADIUS - ARC_WIDTH - 10) : (RADIUS - ARC_WIDTH - 8);
      int outerRadius = isMajorTick ? (RADIUS - ARC_WIDTH - 5)  : (RADIUS - ARC_WIDTH - 7);

      int lineX1 = CENTER_X_CORRECTED + (cos(rad) * innerRadius);
      int lineY1 = CENTER_Y_CORRECTED - (sin(rad) * innerRadius);
      int lineX2 = CENTER_X_CORRECTED + (cos(rad) * outerRadius);
      int lineY2 = CENTER_Y_CORRECTED - (sin(rad) * outerRadius);

      canvas.drawLine(lineX1, lineY1, lineX2, lineY2, COLOR_WHITE);

      if (isMajorTick) {
        int labelX = CENTER_X_CORRECTED + (cos(rad) * (RADIUS - ARC_WIDTH - 15));
        int labelY = CENTER_Y_CORRECTED - (sin(rad) * (RADIUS - ARC_WIDTH - 15));

        char labelText[6];
        snprintf(labelText, sizeof(labelText), "%.0f", scaledValue);

        canvas.setTextFont(1);
        canvas.setFont(&fonts::Font0);
        if (fabsf(scaledValue - 90.0f) < 0.1f || fabsf(scaledValue - 100.0f) < 0.1f)
          canvas.setTextSize(2);
        else
          canvas.setTextSize(1);
        canvas.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
        canvas.setCursor(labelX - (canvas.textWidth(labelText) / 2), labelY - 4);
        canvas.print(labelText);
      }
    }

    char combinedLabel[30];
    snprintf(combinedLabel, sizeof(combinedLabel), "%s / %s", label, unit);
    canvas.setFont(&fonts::Font0);
    int labelX = CENTER_X_CORRECTED;
    int labelY = CENTER_Y_CORRECTED + RADIUS + 15;
    canvas.setCursor(labelX - (canvas.textWidth(combinedLabel) / 2), labelY);
    canvas.print(combinedLabel);
  }

  char valueText[10];
  if (useDecimal) {
    snprintf(valueText, sizeof(valueText), "%.1f", value);
  } else {
    snprintf(valueText, sizeof(valueText), "%.0f", round(value));
  }

  canvas.setFont(&FreeSansBold24pt7b);
  int valueX = VALUE_BASE_X;
  int valueY = CENTER_Y_CORRECTED + RADIUS - 20;
  canvas.fillRect(valueX - 75, valueY - canvas.fontHeight() / 2 - 2,
                  75, canvas.fontHeight() + 4, BACKGROUND_COLOR);
  canvas.setCursor(valueX - canvas.textWidth(valueText), valueY - (canvas.fontHeight() / 2));
  canvas.print(valueText);
}

