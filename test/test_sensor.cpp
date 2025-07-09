#include <cassert>
#include <cmath>

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
  // 0,3,5,7,10bar に相当する電圧で検証
  float pressures[] = {0.5f, 1.7f, 2.5f, 3.3f, 4.5f};
  float expectedPressures[] = {0.0f, 3.0f, 5.0f, 7.0f, 10.0f};
  for (int i = 0; i < 5; ++i) {
    float p = convertVoltageToOilPressure(pressures[i]);
    assert(std::abs(std::abs(p) - expectedPressures[i]) < 0.01f);
  }

  // ---- 温度変換の追加テスト ----
  // 100℃,90℃,80℃,70℃に相当する電圧で確認
  float temps[] = {0.4646f, 0.5810f, 0.7305f, 0.9222f};
  float expectedTemps[] = {100.0f, 90.0f, 80.0f, 70.0f};
  for (int i = 0; i < 4; ++i) {
    float t = convertVoltageToTemp(temps[i]);
    assert(std::abs(std::abs(t) - expectedTemps[i]) < 0.5f);
  }

  float values[3] = {1.0f, 2.0f, 3.0f};
  assert(std::abs(calculateAverage(values) - 2.0f) < 0.001f);

  return 0;
}
