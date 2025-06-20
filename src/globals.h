#pragma once
#include <M5GFX.h>
#include <Adafruit_ADS1X15.h>
#include "config.h"
#include <limits>

// ── ALS 関連 ──
extern BrightnessMode currentBrightnessMode;
extern uint32_t luxSampleBuffer[MEDIAN_BUFFER_SIZE];
extern int      luxSampleIndex;

// ── M5Stack 画面関連 ──
extern M5GFX    display;
extern M5Canvas mainCanvas;

// ── ADS1015 ──
extern Adafruit_ADS1015 adsConverter;

// ── センサリング用バッファ ──

extern float oilPressureSamples[PRESSURE_SAMPLE_SIZE];
extern float waterTemperatureSamples[WATER_TEMP_SAMPLE_SIZE];
extern float oilTemperatureSamples[OIL_TEMP_SAMPLE_SIZE];

extern int oilPressureSampleIndex;
extern int waterTemperatureSampleIndex;
extern int oilTemperatureSampleIndex;

extern float recordedMaxOilPressure;
extern float recordedMaxWaterTemp;
extern int   recordedMaxOilTempTop;

extern bool pressureGaugeInitialized;
extern bool waterGaugeInitialized;

extern int currentFramesPerSecond;

struct DisplayCache {
  float pressureAvg;
  float waterTempAvg;
  float oilTemp;
  int16_t maxOilTemp;
};
extern DisplayCache displayCache;

