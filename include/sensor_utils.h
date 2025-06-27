#ifndef SENSOR_UTILS_H
#define SENSOR_UTILS_H

#include <cmath>
#include <cstdint>

// ───────────────── 変換定数 ─────────────────
constexpr float SUPPLY_VOLTAGE          = 5.0f;
constexpr float THERMISTOR_R25          = 10000.0f;
constexpr float THERMISTOR_B_CONSTANT   = 3380.0f;
constexpr float ABSOLUTE_TEMPERATURE_25 = 298.16f;       // 273.16 + 25
constexpr float SERIES_REFERENCE_RES    = 10000.0f;

// ───────────────── ユーティリティ関数 ─────────────────
inline float convertAdcToVoltage(int16_t rawAdc)
{
    return (rawAdc * 6.144f) / 2047.0f;
}

inline float convertVoltageToOilPressure(float voltage)
{
    return (voltage > 0.5f) ? 2.5f * (voltage - 0.5f) : 0.0f;
}

inline float convertVoltageToTemp(float voltage)
{
    if (voltage <= 0.0f) return 200.0f;
    float resistance = SERIES_REFERENCE_RES * ((SUPPLY_VOLTAGE / voltage) - 1.0f);
    float kelvin = THERMISTOR_B_CONSTANT /
                   (log(resistance / THERMISTOR_R25) + THERMISTOR_B_CONSTANT / ABSOLUTE_TEMPERATURE_25);
    return std::isnan(kelvin) ? 200.0f : kelvin - 273.16f;
}

template <size_t N>
inline float calculateAverage(const float (&values)[N])
{
    float sum = 0.0f;
    for (size_t i = 0; i < N; ++i) {
        sum += values[i];
    }
    return sum / static_cast<float>(N);
}

#endif // SENSOR_UTILS_H
