#ifndef ADAFRUIT_ADS1X15_H
#define ADAFRUIT_ADS1X15_H
#include <stdint.h>
#define RATE_ADS1015_1600SPS 0
class Adafruit_ADS1015
{
 public:
  bool begin() { return true; }
  void setDataRate(int) {}

  // テスト用にチャネルごとのADC値を設定できるようにする
  static void setMockValue(uint8_t ch, int16_t value) { mockValues[ch] = value; }

  int16_t readADC_SingleEnded(uint8_t ch) { return mockValues[ch]; }

 private:
static int16_t mockValues[4];
};

// モック値の初期化
inline int16_t Adafruit_ADS1015::mockValues[4] = {0};
#endif
