#ifndef SENSOR_CONVERSION_H
#define SENSOR_CONVERSION_H

#include <cmath>
#include <cstdint>

float adc_to_oil_press(int adc);
float adc_to_water_temp(int adc);
float adc_to_oil_temp(int adc);

#endif  // SENSOR_CONVERSION_H
