#ifndef FAST_TRIG_H
#define FAST_TRIG_H

#include <cmath>

// 三角関数の計算結果を事前計算しておくことで計算負荷を下げる
inline float fastSin(float deg)
{
  static bool   initialized = false;
  static float  table[361];
  if (!initialized)
  {
    for (int i = 0; i <= 360; ++i)
    {
      table[i] = sinf(i * M_PI / 180.0f);
    }
    initialized = true;
  }
  int idx = static_cast<int>(deg + 0.5f) % 360;
  if (idx < 0) idx += 360;
  return table[idx];
}

inline float fastCos(float deg)
{
  // cos θ = sin(θ + 90°) を利用
  return fastSin(deg + 90.0f);
}

#endif // FAST_TRIG_H
