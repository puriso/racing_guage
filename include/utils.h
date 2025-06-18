#ifndef UTILS_H
#define UTILS_H

#include <cmath>
#include <numeric>
#include <limits>
#include <cstdint>

// ────────────────────── 電圧→物理量変換定数 ──────────────────────
constexpr float SUPPLY_VOLTAGE          = 5.0f;
constexpr float THERMISTOR_R25          = 10000.0f;
constexpr float THERMISTOR_B_CONSTANT   = 3380.0f;
constexpr float ABSOLUTE_TEMPERATURE_25 = 298.16f;       // 273.16 + 25
constexpr float SERIES_REFERENCE_RES    = 10000.0f;

// ────────────────────── ユーティリティ関数 ──────────────────────
inline float convertAdcToVoltage(int16_t rawAdc)
{
    return (rawAdc * 6.144f) / 2047.0f;
}

inline float convertVoltageToOilPressure(float voltage)
{
    return (voltage > 0.5f) ? 2.5f * (voltage - 0.5f) : 0.0f;   // 0.5 V offset, 2.5 bar/V
}

inline float convertVoltageToTemp(float voltage)
{
    if (voltage <= 0.0f) return 200.0f;  // ゼロ除算回避
    float resistance = SERIES_REFERENCE_RES * ((SUPPLY_VOLTAGE / voltage) - 1.0f);
    float kelvin     = THERMISTOR_B_CONSTANT /
                       (std::log(resistance / THERMISTOR_R25) + THERMISTOR_B_CONSTANT / ABSOLUTE_TEMPERATURE_25);
    return std::isnan(kelvin) ? 200.0f : kelvin - 273.16f;  // Kelvin→℃ 変換
}

template <size_t N>
inline float calculateAverage(const float (&values)[N])
{
    float sum = std::accumulate(values, values + N, 0.0f);
    return sum / static_cast<float>(N);
}

#endif // UTILS_H
