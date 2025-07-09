#ifndef ADAFRUIT_ADS1X15_H
#define ADAFRUIT_ADS1X15_H
#include <stdint.h>
#define RATE_ADS1015_1600SPS 0
class Adafruit_ADS1015
{
 public:
  bool begin() { return true; }
  void setDataRate(int) {}
  int16_t readADC_SingleEnded(uint8_t) { return 0; }
};
#endif
