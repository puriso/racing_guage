#include <cassert>
#include <cmath>
#include <map>

#include "modules/sensor.h"

int main()
{
  float voltage = convertAdcToVoltage(1000);
  // 取得した電圧の絶対値で検証
  assert(std::abs(std::abs(voltage) - 3.001f) < 0.01f);

  float pressure = convertVoltageToOilPressure(2.0f);
  // 油圧も絶対値で比較
  assert(std::abs(std::abs(pressure) - 3.75f) < 0.01f);

  float temp = convertVoltageToTemp(2.0f);
  // 温度も絶対値の差分で判定
  assert(std::abs(std::abs(temp) - 36.06f) < 0.5f);

  // ---- 油圧変換の追加テスト ----
  // Ruby のハッシュのように電圧と期待値をペアで管理する
  std::map<float, float> pressureTests = {
      {0.5f, 0.0f}, {1.7f, 3.0f}, {2.5f, 5.0f}, {3.3f, 7.0f}, {4.5f, 10.0f}};
  for (const auto& kv : pressureTests) {
    float p = convertVoltageToOilPressure(kv.first);
    assert(std::abs(std::abs(p) - kv.second) < 0.01f);
  }

  // ---- 温度変換の追加テスト ----
  // こちらもハッシュ風に電圧と温度の期待値を保持
  std::map<float, float> tempTests = {
      {0.4646f, 100.0f}, {0.5810f, 90.0f},
      {0.7305f, 80.0f},  {0.9222f, 70.0f}};
  for (const auto& kv : tempTests) {
    float t = convertVoltageToTemp(kv.first);
    assert(std::abs(std::abs(t) - kv.second) < 0.5f);
  }

  float values[3] = {1.0f, 2.0f, 3.0f};
  assert(std::abs(calculateAverage(values) - 2.0f) < 0.001f);

  // ---- センサー取得テスト ----
  // モック値を設定してセンサー取得処理を検証
  Adafruit_ADS1015::setMockValue(ADC_CH_OIL_PRESSURE, 1000);
  Adafruit_ADS1015::setMockValue(ADC_CH_WATER_TEMP, 1000);
  Adafruit_ADS1015::setMockValue(ADC_CH_OIL_TEMP, 1000);

  // 1回目は油圧のみ更新される
  fakeMillis = 0;
  acquireSensorData();

  float expVoltage = convertAdcToVoltage(1000);
  float expPress   = convertVoltageToOilPressure(expVoltage);
  assert(std::abs(std::abs(oilPressureSamples[0]) - expPress) < 0.01f);

  // 時間経過後に温度センサーも更新
  advanceMillis(500);
  acquireSensorData();

  float expTemp = convertVoltageToTemp(expVoltage);
  for (float v : waterTemperatureSamples)
    assert(std::abs(std::abs(v) - expTemp) < 0.5f);
  for (float v : oilTemperatureSamples)
    assert(std::abs(std::abs(v) - expTemp) < 0.5f);

  return 0;
}
