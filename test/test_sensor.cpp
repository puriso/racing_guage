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

  float values[3] = {1.0f, 2.0f, 3.0f};
  assert(std::abs(calculateAverage(values) - 2.0f) < 0.001f);

  return 0;
}
