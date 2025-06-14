#ifndef CONVERSION_H
#define CONVERSION_H

// 変換ヘルパー関数群

#include <cmath>
#include <cstdint>

constexpr float SUPPLY_VOLTAGE = 5.0f;
constexpr float THERMISTOR_R25 = 10000.0f;
constexpr float THERMISTOR_B_CONSTANT = 3380.0f;
constexpr float ABSOLUTE_TEMPERATURE_25 = 298.15f;
constexpr float SERIES_REFERENCE_RES = 10000.0f;

// ADC 値を電圧 (V) に変換
inline float convertAdcToVoltage(int16_t rawAdc) { return (rawAdc * 6.144f) / 2047.0f; }

// 電圧を油圧 (bar) に変換
inline float convertVoltageToOilPressure(float voltage) { return (voltage > 0.5f) ? 2.5f * (voltage - 0.5f) : 0.0f; }

// 電圧を温度 (°C) に変換
inline float convertVoltageToTemp(float voltage)
{
  float resistance = SERIES_REFERENCE_RES * ((SUPPLY_VOLTAGE / voltage) - 1.0f);
  float kelvin =
      THERMISTOR_B_CONSTANT / (log(resistance / THERMISTOR_R25) + THERMISTOR_B_CONSTANT / ABSOLUTE_TEMPERATURE_25);
  return std::isnan(kelvin) ? 200.0f : kelvin - 273.15f;
}

#endif  // CONVERSION_H
