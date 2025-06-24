#include "sensor.h"
#include "sensor_utils.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <Wire.h>

// ────────────────────── グローバル変数 ──────────────────────
Adafruit_ADS1015 adsConverter;

float oilPressureSamples[PRESSURE_SAMPLE_SIZE] = {};
float waterTemperatureSamples[WATER_TEMP_SAMPLE_SIZE] = {};
float oilTemperatureSamples[OIL_TEMP_SAMPLE_SIZE] = {};

static int oilPressureSampleIndex = 0;
static int waterTemperatureSampleIndex = 0;
static int oilTemperatureSampleIndex = 0;

// 最初の水温・油温取得かどうかのフラグ
static bool waterTempFirstSample = true;
static bool oilTempFirstSample   = true;

// セトリング用待ち時間 [us]
constexpr uint32_t ADC_SETTLING_US = 50;

// 水温・油温サンプリング間隔 [ms]
constexpr uint32_t TEMP_SAMPLE_INTERVAL_MS = 300;


// ────────────────────── ADC 読み取り ──────────────────────
int16_t readAdcWithSettling(uint8_t ch)
{
    adsConverter.readADC_SingleEnded(ch);  // ダミー変換
    delayMicroseconds(ADC_SETTLING_US);    // セトリング待ち
    return adsConverter.readADC_SingleEnded(ch);
}

// ────────────────────── センサ取得 ──────────────────────
void acquireSensorData()
{
    static unsigned long previousWaterTempSampleTime = 0;
    static unsigned long previousOilTempSampleTime = 0;
    unsigned long now = millis();

    // 油圧
    if (SENSOR_OIL_PRESSURE_PRESENT) {
        int16_t raw = readAdcWithSettling(ADC_CH_OIL_PRESSURE);  // CH1: 油圧
        oilPressureSamples[oilPressureSampleIndex] =
            convertVoltageToOilPressure(convertAdcToVoltage(raw));
    } else {
        oilPressureSamples[oilPressureSampleIndex] = 0.0f;
    }
    oilPressureSampleIndex = (oilPressureSampleIndex + 1) % PRESSURE_SAMPLE_SIZE;

    // 水温
    if (now - previousWaterTempSampleTime >= TEMP_SAMPLE_INTERVAL_MS) {
        float value = 0.0f;
        if (SENSOR_WATER_TEMP_PRESENT) {
            int16_t raw = readAdcWithSettling(ADC_CH_WATER_TEMP);  // CH0: 水温
            value = convertVoltageToTemp(convertAdcToVoltage(raw));
        }

        if (waterTempFirstSample) {
            for (float& v : waterTemperatureSamples) v = value;  // 初期値を全要素に設定
            waterTemperatureSampleIndex = 1 % WATER_TEMP_SAMPLE_SIZE;
            waterTempFirstSample = false;
        } else {
            waterTemperatureSamples[waterTemperatureSampleIndex] = value;
            waterTemperatureSampleIndex =
                (waterTemperatureSampleIndex + 1) % WATER_TEMP_SAMPLE_SIZE;
        }
        previousWaterTempSampleTime = now;
    }

    // 油温
    if (now - previousOilTempSampleTime >= TEMP_SAMPLE_INTERVAL_MS) {
        float value = 0.0f;
        if (SENSOR_OIL_TEMP_PRESENT) {
            int16_t raw = readAdcWithSettling(ADC_CH_OIL_TEMP);  // CH2: 油温
            value = convertVoltageToTemp(convertAdcToVoltage(raw));
        }

        if (oilTempFirstSample) {
            for (float& v : oilTemperatureSamples) v = value;  // 初期値を全要素に設定
            oilTemperatureSampleIndex = 1 % OIL_TEMP_SAMPLE_SIZE;
            oilTempFirstSample = false;
        } else {
            oilTemperatureSamples[oilTemperatureSampleIndex] = value;
            oilTemperatureSampleIndex =
                (oilTemperatureSampleIndex + 1) % OIL_TEMP_SAMPLE_SIZE;
        }
        previousOilTempSampleTime = now;
    }
}

