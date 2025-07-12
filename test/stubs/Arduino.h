#ifndef ARDUINO_H
#define ARDUINO_H
#include <stdint.h>
inline void delayMicroseconds(unsigned int us) {}
inline unsigned long millis() { return 0; }
#endif
