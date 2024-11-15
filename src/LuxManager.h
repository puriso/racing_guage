#ifndef LUX_MANAGER_H
#define LUX_MANAGER_H

#include <M5CoreS3.h>

const int sampleCount = 20;      // N回分のサンプルを取得して平均を計算

class LuxManager {
private:
    float luxSamples[sampleCount];
    int currentSample;

public:
    void initializeLuxSamples() {
        for (int i = 0; i < sampleCount; i++) {
            luxSamples[i] = CoreS3.Ltr553.getAlsValue();
        }
    }

    void updateLuxSamples() {
        float currentLux = CoreS3.Ltr553.getAlsValue(); // LTR-553から照度を取得
        // 末尾に追加
        luxSamples[currentSample] = currentLux;

        luxSamples[currentSample] = currentLux;
        currentSample = (currentSample + 1) % sampleCount; // リングバッファ

        Serial.printf("Lux Sample %d: %.2f lx\n", currentSample, currentLux);
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
