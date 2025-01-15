#ifndef LUX_MANAGER_H
#define LUX_MANAGER_H

#include <M5CoreS3.h>

const int sampleCount = 70;  // サンプル数

class LuxManager
{
 private:
  float luxSamples[sampleCount];
  int currentSample;

 public:
  // コンストラクタ
  LuxManager() : currentSample(0) {}

  // 照度サンプルの初期化
  void initializeLuxSamples()
  {
    for (int i = 0; i < sampleCount; i++)
    {
      luxSamples[i] = CoreS3.Ltr553.getAlsValue();
    }
    currentSample = 0;  // currentSample を初期化
  }

  // 照度サンプルの更新
  void updateLuxSamples()
  {
    float currentLux = CoreS3.Ltr553.getAlsValue();  // LTR-553から照度を取得

    // リングバッファに追加
    luxSamples[currentSample] = currentLux;
    currentSample = (currentSample + 1) % sampleCount;  // リングバッファのインデックス更新

    Serial.printf("Updated Lux Sample %d: %.2f lx\n", currentSample, currentLux);

    // 配列の内容をログに出力
    ///logLuxSamples();
  }

  // 平均照度を計算
  float calculateAverageLux()
  {
    float sumLux = 0;
    for (int i = 0; i < sampleCount; i++)
    {
      sumLux += luxSamples[i];
    }
    return sumLux / sampleCount;
  }

  // 配列内容をログに出力
  void logLuxSamples()
  {
    Serial.println("Current Lux Samples:");
    for (int i = 0; i < sampleCount; i++)
    {
      Serial.printf("Index %d: %.2f lx\n", i, luxSamples[i]);
    }
    Serial.println("----------------------");
  }
};

#endif
