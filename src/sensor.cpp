#include "sensor.h"
#include <Wire.h>

int16_t readAdcWithSettling(uint8_t ch) {
  adsConverter.readADC_SingleEnded(ch);  // ダミー変換
  delayMicroseconds(ADC_SETTLING_US);     // セトリング待ち
  return adsConverter.readADC_SingleEnded(ch);
}

void acquireSensorData() {
  static unsigned long previousWaterTempSampleTime = 0;
  static unsigned long previousOilTempSampleTime   = 0;
  unsigned long now = millis();

  // 油圧
  if (SENSOR_OIL_PRESSURE_PRESENT) {
    int16_t raw = readAdcWithSettling(1);  // CH1: 油圧
    oilPressureSamples[oilPressureSampleIndex] =
        convertVoltageToOilPressure(convertAdcToVoltage(raw));
  } else {
    oilPressureSamples[oilPressureSampleIndex] = 0.0f;
  }
  oilPressureSampleIndex = (oilPressureSampleIndex + 1) % PRESSURE_SAMPLE_SIZE;

  // 水温
  if (now - previousWaterTempSampleTime >= TEMP_SAMPLE_INTERVAL_MS) {
    if (SENSOR_WATER_TEMP_PRESENT) {
      int16_t raw = readAdcWithSettling(0);  // CH0: 水温
      waterTemperatureSamples[waterTemperatureSampleIndex] =
          convertVoltageToTemp(convertAdcToVoltage(raw));
    } else {
      waterTemperatureSamples[waterTemperatureSampleIndex] = 0.0f;
    }
    waterTemperatureSampleIndex = (waterTemperatureSampleIndex + 1) % WATER_TEMP_SAMPLE_SIZE;
    previousWaterTempSampleTime = now;
  }

  // 油温
  if (now - previousOilTempSampleTime >= TEMP_SAMPLE_INTERVAL_MS) {
    if (SENSOR_OIL_TEMP_PRESENT) {
      int16_t raw = readAdcWithSettling(2);  // CH2: 油温
      oilTemperatureSamples[oilTemperatureSampleIndex] =
          convertVoltageToTemp(convertAdcToVoltage(raw));
    } else {
      oilTemperatureSamples[oilTemperatureSampleIndex] = 0.0f;
    }
    oilTemperatureSampleIndex = (oilTemperatureSampleIndex + 1) % OIL_TEMP_SAMPLE_SIZE;
    previousOilTempSampleTime = now;
  }
}
