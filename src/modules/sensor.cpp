#include "sensor.h"

#include <Wire.h>

#include <algorithm>
#include <cmath>
#include <numeric>

// ────────────────────── グローバル変数 ──────────────────────
Adafruit_ADS1015 adsConverter;

float oilPressureSamples[PRESSURE_SAMPLE_SIZE] = {};
float oilPressureVoltageSamples[PRESSURE_SAMPLE_SIZE] = {};
float waterTemperatureSamples[WATER_TEMP_SAMPLE_SIZE] = {};
float waterTempVoltageSamples[WATER_TEMP_SAMPLE_SIZE] = {};
float oilTemperatureSamples[OIL_TEMP_SAMPLE_SIZE] = {};
float oilTempVoltageSamples[OIL_TEMP_SAMPLE_SIZE] = {};
static int oilPressureIndex = 0;
static int waterTempIndex = 0;
static int oilTempIndex = 0;

// 最初の水温・油温取得かどうかのフラグ
static bool isFirstWaterTempSample = true;
static bool isFirstOilTempSample = true;

// ADC セトリング待ち時間 [us]
constexpr uint16_t ADC_SETTLING_US = 50;

// 温度サンプリング間隔 [ms]
// 500msごとに取得し、10サンプルで約5秒平均となる
constexpr uint16_t TEMP_SAMPLE_INTERVAL_MS = 500;

// ────────────────────── 変換定数 ──────────────────────
constexpr float SUPPLY_VOLTAGE = 5.0f;
constexpr float THERMISTOR_R25 = 10000.0f;
constexpr float THERMISTOR_B_CONSTANT = 3380.0f;
constexpr float ABSOLUTE_TEMPERATURE_25 = 298.16f;  // 273.16 + 25
constexpr float SERIES_REFERENCE_RES = 10000.0f;

// ────────────────────── ユーティリティ ──────────────────────
static float convertAdcToVoltage(int16_t rawAdc) { return (rawAdc * 6.144f) / 2047.0f; }

static float convertVoltageToOilPressure(float voltage)
{
  // 電源電圧近くまで上昇してもそのまま変換し、
  // 12bar 以上かどうかは呼び出し側で判断する

  // センサー実測式に基づき圧力へ変換
  return (voltage > 0.5f) ? 2.5f * (voltage - 0.5f) : 0.0f;
}

static float convertVoltageToTemp(float voltage)
{
  // 電源電圧より高い/等しい電圧は異常値として捨てる
  if (voltage <= 0.0f || voltage >= SUPPLY_VOLTAGE) return 200.0f;

  // 分圧式よりサーミスタ抵抗値を算出
  // R = Rref * (V / (Vcc - V))  (サーミスタがGND側の場合)
  float resistance = SERIES_REFERENCE_RES * (voltage / (SUPPLY_VOLTAGE - voltage));

  // Steinhart–Hart の簡易形 (β式)
  float kelvin =
      THERMISTOR_B_CONSTANT / (log(resistance / THERMISTOR_R25) + THERMISTOR_B_CONSTANT / ABSOLUTE_TEMPERATURE_25);

  return std::isnan(kelvin) ? 200.0f : kelvin - 273.16f;
}

// ────────────────────── ADC 読み取り ──────────────────────
static int16_t readAdcWithSettling(uint8_t ch)
{
  adsConverter.readADC_SingleEnded(ch);  // ダミー変換
  delayMicroseconds(ADC_SETTLING_US);    // セトリング待ち
  return adsConverter.readADC_SingleEnded(ch);
}

// ────────────────────── 温度読み取り ──────────────────────
// 指定チャンネルから温度を取得して摂氏に変換
static float readTemperatureChannel(uint8_t ch)
{
  int16_t raw = readAdcWithSettling(ch);
  return convertVoltageToTemp(convertAdcToVoltage(raw));
}

// ────────────────────── センサ取得 ──────────────────────
void acquireSensorData()
{
  static unsigned long lastWaterTempSampleTime = 0;
  static unsigned long lastOilTempSampleTime = 0;
  unsigned long now = millis();

  // 油圧
  if (SENSOR_OIL_PRESSURE_PRESENT)
  {
    int16_t rawAdc = readAdcWithSettling(ADC_CH_OIL_PRESSURE);  // CH1: 油圧
    float voltage = convertAdcToVoltage(rawAdc);
    float pressureValue = convertVoltageToOilPressure(voltage);
    oilPressureSamples[oilPressureIndex] = pressureValue;
    oilPressureVoltageSamples[oilPressureIndex] = voltage;
  }
  else
  {
    oilPressureSamples[oilPressureIndex] = 0.0f;
    oilPressureVoltageSamples[oilPressureIndex] = 0.0f;
  }
  oilPressureIndex = (oilPressureIndex + 1) % PRESSURE_SAMPLE_SIZE;

  // 水温
  if (now - lastWaterTempSampleTime >= TEMP_SAMPLE_INTERVAL_MS)
  {
    float voltage = SENSOR_WATER_TEMP_PRESENT ? convertAdcToVoltage(readAdcWithSettling(ADC_CH_WATER_TEMP)) : 0.0f;
    float value = SENSOR_WATER_TEMP_PRESENT ? convertVoltageToTemp(voltage) : 0.0f;
    if (isFirstWaterTempSample)
    {
      for (float &v : waterTemperatureSamples) v = value;
      for (float &v : waterTempVoltageSamples) v = voltage;
      waterTempIndex = 1 % WATER_TEMP_SAMPLE_SIZE;
      isFirstWaterTempSample = false;
    }
    else
    {
      waterTemperatureSamples[waterTempIndex] = value;
      waterTempVoltageSamples[waterTempIndex] = voltage;
      waterTempIndex = (waterTempIndex + 1) % WATER_TEMP_SAMPLE_SIZE;
    }
    lastWaterTempSampleTime = now;
  }

  // 油温
  if (now - lastOilTempSampleTime >= TEMP_SAMPLE_INTERVAL_MS)
  {
    float voltage = SENSOR_OIL_TEMP_PRESENT ? convertAdcToVoltage(readAdcWithSettling(ADC_CH_OIL_TEMP)) : 0.0f;
    float value = SENSOR_OIL_TEMP_PRESENT ? convertVoltageToTemp(voltage) : 0.0f;
    if (isFirstOilTempSample)
    {
      for (float &v : oilTemperatureSamples) v = value;
      for (float &v : oilTempVoltageSamples) v = voltage;
      oilTempIndex = 1 % OIL_TEMP_SAMPLE_SIZE;
      isFirstOilTempSample = false;
    }
    else
    {
      oilTemperatureSamples[oilTempIndex] = value;
      oilTempVoltageSamples[oilTempIndex] = voltage;
      oilTempIndex = (oilTempIndex + 1) % OIL_TEMP_SAMPLE_SIZE;
    }
    lastOilTempSampleTime = now;
  }
}
