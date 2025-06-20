#ifndef DRAW_FILL_ARC_METER_H
#define DRAW_FILL_ARC_METER_H

#include <M5GFX.h>
#include <algorithm>
#include <cmath>

// 半円メーターを描画する
void drawFillArcMeter(M5Canvas& canvas, float value, float minValue, float maxValue,
                      float threshold, uint16_t overThresholdColor, const char* unit,
                      const char* label, float& maxRecordedValue, float tickStep,
                      bool useDecimal, int x, int y, bool drawStatic,
                      float majorTickStep = -1.0f, float labelStart = 0.0f);

#endif  // DRAW_FILL_ARC_METER_H
