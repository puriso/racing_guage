#pragma once
#include <cstdint>
#include <cmath>
#include <numeric>
#include "globals.h"

// ── ユーティリティ ──
inline float convertAdcToVoltage(int16_t rawAdc) {
  return (rawAdc * 6.144f) / 2047.0f;
}

inline float convertVoltageToOilPressure(float voltage) {
  return (voltage > 0.5f) ? 2.5f * (voltage - 0.5f) : 0.0f;
}

inline float convertVoltageToTemp(float voltage) {
  if (voltage <= 0.0f) return 200.0f;
  constexpr float SUPPLY_VOLTAGE          = 5.0f;
  constexpr float THERMISTOR_R25          = 10000.0f;
  constexpr float THERMISTOR_B_CONSTANT   = 3380.0f;
  constexpr float ABSOLUTE_TEMPERATURE_25 = 298.16f;
  constexpr float SERIES_REFERENCE_RES    = 10000.0f;
  float resistance = SERIES_REFERENCE_RES * ((SUPPLY_VOLTAGE / voltage) - 1.0f);
  float kelvin = THERMISTOR_B_CONSTANT /
      (log(resistance / THERMISTOR_R25) + THERMISTOR_B_CONSTANT / ABSOLUTE_TEMPERATURE_25);
  return std::isnan(kelvin) ? 200.0f : kelvin - 273.16f;
}

template <size_t N>
inline float calculateAverage(const float (&values)[N]) {
  float sum = std::accumulate(values, values + N, 0.0f);
  return sum / static_cast<float>(N);
}

int16_t readAdcWithSettling(uint8_t ch);
void acquireSensorData();
