#include "sensor.h"

#include <Wire.h>

#include <algorithm>
#include <cmath>
#include <numeric>

// ────────────────────── グローバル変数 ──────────────────────
Adafruit_ADS1015 adsConverter;

float oilPressureSamples[PRESSURE_SAMPLE_SIZE] = {};
float waterTemperatureSamples[WATER_TEMP_SAMPLE_SIZE] = {};
float oilTemperatureSamples[OIL_TEMP_SAMPLE_SIZE] = {};
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
// 電圧降下は config で設定
constexpr float CORRECTION_FACTOR = SUPPLY_VOLTAGE / (SUPPLY_VOLTAGE - VOLTAGE_DROP);
constexpr float THERMISTOR_R25 = 10000.0f;
constexpr float THERMISTOR_B_CONSTANT = 3380.0f;
constexpr float ABSOLUTE_TEMPERATURE_25 = 298.16f;  // 273.16 + 25
constexpr float SERIES_REFERENCE_RES = 10000.0f;

// ────────────────────── ユーティリティ ──────────────────────
static float convertAdcToVoltage(int16_t rawAdc) { return (rawAdc * 6.144f) / 2047.0f; }

static float convertVoltageToOilPressure(float voltage)
{
  voltage *= CORRECTION_FACTOR;
  // 電源電圧近くまで上昇してもそのまま変換し、
  // 12bar 以上かどうかは呼び出し側で判断する

  // センサー実測式に基づき圧力へ変換
  return (voltage > 0.5f) ? 2.5f * (voltage - 0.5f) : 0.0f;
}

static float convertVoltageToTemp(float voltage)
{
  voltage *= CORRECTION_FACTOR;
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

// ────────────────────── サンプルバッファ更新 ──────────────────────
// 初回は全要素を同じ値で埋め、その後はリングバッファ更新
template <size_t N>
static void updateSampleBuffer(float value, float (&buffer)[N], int &index, bool &first)
{
  if (first)
  {
    for (float &v : buffer) v = value;
    index = 1 % N;  // 初期化後は 1 番目から開始
    first = false;
  }
  else
  {
    buffer[index] = value;
    index = (index + 1) % N;
  }
}

// ────────────────────── センサ取得 ──────────────────────
void acquireSensorData()
{
  static unsigned long lastWaterTempSampleTime = 0;
  static unsigned long lastOilTempSampleTime = 0;

  // デモモード用の変数
  // デモ用電圧とシーケンス管理変数
  static float demoVoltage = 0.0f;    // 現在のデモ電圧
  static unsigned long demoTick = 0;  // 更新タイマ
  static bool inPattern = false;      // 0→5V上昇後のパターンフェーズか
  static size_t patternIndex = 0;     // パターンインデックス
  // デモモードでの電圧変化パターン
  // 5V到達後に 5,0,5,4,3,2,1,0,1,2,3,4,5,0,0,2.5 と0.5秒ごとに変化させる
  constexpr float patternSeq[] = {5.0f, 0.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f,
                                  1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 0.0f, 0.0f, 2.5f};

  unsigned long now = millis();

  // デモモード処理
  if (DEMO_MODE_ENABLED)
  {
    // 上昇フェーズ
    if (!inPattern)
    {
      if (now - demoTick >= 1000)
      {
        demoVoltage += 0.25f;
        if (demoVoltage >= 5.0f)
        {
          demoVoltage = 5.0f;
          inPattern = true;
          patternIndex = 0;
        }
        demoTick = now;
      }
    }
    else
    {
      // パターンフェーズ
      if (now - demoTick >= 500)
      {
        demoVoltage = patternSeq[patternIndex];
        patternIndex++;
        if (patternIndex >= sizeof(patternSeq) / sizeof(patternSeq[0]))
        {
          patternIndex = 0;
          inPattern = false;
          demoVoltage = 0.0f;
        }
        demoTick = now;
      }
    }

    float demoPressure = convertVoltageToOilPressure(demoVoltage);
    // 温度センサは電圧変化と逆の振る舞いにする
    float demoTemp = convertVoltageToTemp(SUPPLY_VOLTAGE - demoVoltage);

    oilPressureSamples[oilPressureIndex] = demoPressure;
    updateSampleBuffer(demoTemp, waterTemperatureSamples, waterTempIndex, isFirstWaterTempSample);
    updateSampleBuffer(demoTemp, oilTemperatureSamples, oilTempIndex, isFirstOilTempSample);

    Serial.printf("[DEMO] V:%.2f P:%.2f T:%.1f\n", demoVoltage, demoPressure, demoTemp);

    oilPressureIndex = (oilPressureIndex + 1) % PRESSURE_SAMPLE_SIZE;
    return;
  }

  // ── 通常センサ読み取り ──
  if (SENSOR_OIL_PRESSURE_PRESENT)
  {
    int16_t rawAdc = readAdcWithSettling(ADC_CH_OIL_PRESSURE);  // CH1: 油圧
    float pressureValue = convertVoltageToOilPressure(convertAdcToVoltage(rawAdc));
    oilPressureSamples[oilPressureIndex] = pressureValue;
  }
  else
  {
    oilPressureSamples[oilPressureIndex] = 0.0f;
  }
  oilPressureIndex = (oilPressureIndex + 1) % PRESSURE_SAMPLE_SIZE;

  // 水温
  if (now - lastWaterTempSampleTime >= TEMP_SAMPLE_INTERVAL_MS)
  {
    float value = SENSOR_WATER_TEMP_PRESENT ? readTemperatureChannel(ADC_CH_WATER_TEMP) : 0.0f;
    updateSampleBuffer(value, waterTemperatureSamples, waterTempIndex, isFirstWaterTempSample);
    lastWaterTempSampleTime = now;
  }

  // 油温
  if (now - lastOilTempSampleTime >= TEMP_SAMPLE_INTERVAL_MS)
  {
    float value = SENSOR_OIL_TEMP_PRESENT ? readTemperatureChannel(ADC_CH_OIL_TEMP) : 0.0f;
    updateSampleBuffer(value, oilTemperatureSamples, oilTempIndex, isFirstOilTempSample);
    lastOilTempSampleTime = now;
  }
}
