#include "sensor_conversion.h"

// ────────────────────── 定数定義 ──────────────────────
constexpr float SUPPLY_VOLTAGE = 5.0f;  // 電源電圧 [V]
constexpr float ADC_MAX = 4095.0f;      // ADC 最大値

// 水温・油温計算用の二次近似係数
constexpr float TEMP_COEF_A = -5.6666667f;
constexpr float TEMP_COEF_B = 75.8333333f;
constexpr float TEMP_COEF_C = -76.5f;

// ADC 値を電圧へ変換
static inline float adc_to_voltage(int adc) { return (static_cast<float>(adc) / ADC_MAX) * SUPPLY_VOLTAGE; }

// 油圧変換 [bar]
float adc_to_oil_press(int adc)
{
  if (adc < 0 || adc > ADC_MAX) return std::nanf("");

  float voltage = adc_to_voltage(adc);
  if (voltage < 0.5f) return 0.0f;
  return 2.5f * (voltage - 0.5f);
}

// 水温変換 [℃]
float adc_to_water_temp(int adc)
{
  if (adc < 0 || adc > ADC_MAX) return std::nanf("");

  float voltage = adc_to_voltage(adc);
  return TEMP_COEF_A * voltage * voltage + TEMP_COEF_B * voltage + TEMP_COEF_C;
}

// 油温変換 [℃]（水温と同じ計算式）
float adc_to_oil_temp(int adc) { return adc_to_water_temp(adc); }
