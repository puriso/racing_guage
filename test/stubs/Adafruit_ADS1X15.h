#ifndef ADAFRUIT_ADS1X15_H
#define ADAFRUIT_ADS1X15_H
#include <stdint.h>
class Adafruit_ADS1015
{
 public:
  explicit Adafruit_ADS1015(uint8_t addr = 0x48) {}
  int16_t readADC_SingleEnded(uint8_t ch) { return 0; }
};
#endif
