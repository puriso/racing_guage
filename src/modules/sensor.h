#ifndef SENSOR_H
#define SENSOR_H

#include <Adafruit_ADS1X15.h>
#include <stdint.h>
#include "config.h"

extern Adafruit_ADS1015 adsConverter;

extern float oilPressureSamples[PRESSURE_SAMPLE_SIZE];
extern float waterTemperatureSamples[WATER_TEMP_SAMPLE_SIZE];
extern float oilTemperatureSamples[OIL_TEMP_SAMPLE_SIZE];

float convertAdcToVoltage(int16_t rawAdc);
float convertVoltageToOilPressure(float voltage);
float convertVoltageToTemp(float voltage);

int16_t readAdcWithSettling(uint8_t ch);
void acquireSensorData();

// 平均計算テンプレート
template <size_t N>
inline float calculateAverage(const float (&values)[N])
{
    float sum = 0.0f;
    for (size_t i = 0; i < N; ++i) {
        sum += values[i];
    }
    return sum / static_cast<float>(N);
}

// サンプル間の差から値の乱れを検出
template <size_t N>
inline bool isErratic(const float (&values)[N], float threshold)
{
    float minV = values[0];
    float maxV = values[0];
    for (size_t i = 1; i < N; ++i) {
        if (values[i] < minV) minV = values[i];
        if (values[i] > maxV) maxV = values[i];
    }
    return (maxV - minV) > threshold;
}

#endif // SENSOR_H
