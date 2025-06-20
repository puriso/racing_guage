#pragma once
#include <M5GFX.h>
#include "globals.h"
#include "sensor.h"

void drawOilTemperatureTopBar(M5Canvas& canvas, float oilTemp, int maxOilTemp);
void renderDisplayAndLog(float pressureAvg, float waterTempAvg,
                         float oilTemp, int16_t maxOilTemp);
void updateGauges();
