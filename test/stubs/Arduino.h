#ifndef ARDUINO_H
#define ARDUINO_H
#include <stdint.h>
// 最小限の Arduino 互換API
static inline void delayMicroseconds(unsigned int) {}
static inline unsigned long millis() { return 0; }
#endif
