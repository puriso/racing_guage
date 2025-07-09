#ifndef ARDUINO_H
#define ARDUINO_H
#include <stddef.h>
#include <stdint.h>
static inline void delayMicroseconds(unsigned int) {}
static inline unsigned long millis() { return 0; }
#endif
