#ifndef LUX_MANAGER_H
#define LUX_MANAGER_H

#include <M5CoreS3.h>

const int sampleInterval = 500; // Nm秒ごとにサンプリング
const int sampleCount = 20;      // N回分のサンプルを取得して平均を計算
const float initialLux = 200.0;  // 初期照度

class LuxManager {
private:
    float luxSamples[sampleCount];
    int currentSample;

public:
    LuxManager() : currentSample(0) {
        for (int i = 0; i < sampleCount; i++) {
            luxSamples[i] = initialLux; // 初期値を300lxで埋める
        }
    }

    void updateLuxSamples() {
        float currentLux = CoreS3.Ltr553.getAlsValue(); // LTR-553から照度を取得
        luxSamples[currentSample] = currentLux;
        currentSample = (currentSample + 1) % sampleCount; // リングバッファ
    }

    float calculateAverageLux() {
        float sumLux = 0;
        for (int i = 0; i < sampleCount; i++) {
            sumLux += luxSamples[i];
        }
        return sumLux / sampleCount;
    }
};

#endif
