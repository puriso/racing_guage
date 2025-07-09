#ifndef ARDUINO_H
#define ARDUINO_H
#include <stddef.h>
#include <stdint.h>
static inline void delayMicroseconds(unsigned int) {}

// 疑似時間を操作できるようにする
inline unsigned long fakeMillis = 0;
static inline unsigned long millis() { return fakeMillis; }
static inline void advanceMillis(unsigned long ms) { fakeMillis += ms; }
#endif

