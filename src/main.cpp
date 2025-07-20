#include "config.h"

#include <M5CoreS3.h>
#include <Wire.h>
#include <WiFi.h>  // WiFi 無効化用

#include "modules/display.h"
#include "modules/sensor.h"
#include "modules/backlight.h"

// ── FPS 計測用 ──
unsigned long lastFpsTimestamp = 0;  // 直近1秒判定用
int frameCounterPerSecond = 0;
int currentFramesPerSecond = 0;

// ────────────────────── setup() ──────────────────────
void setup()
{
    Serial.begin(115200);

    M5.begin();
    CoreS3.begin(M5.config());

    // WiFi を完全に停止
    WiFi.mode(WIFI_OFF);
    WiFi.disconnect(true);

    // 電源管理を初期化し、処理順序を明確にする
    M5.Power.begin();           // まず電源モジュールを初期化
    M5.Power.setExtOutput(false); // 外部給電時は 5V ピン出力を停止

    display.init();
    // DMA を初期化
    display.initDMA();
    display.setRotation(3);
    display.setColorDepth(16);
    display.setBrightness(BACKLIGHT_DAY);

    mainCanvas.setColorDepth(16);
    mainCanvas.setTextSize(1);
    // スプライトを PSRAM ではなく DMA メモリに確保
    mainCanvas.setPsram(false);
    // スプライト用の DMA を初期化
    mainCanvas.initDMA();
    mainCanvas.createSprite(LCD_WIDTH, LCD_HEIGHT);

    M5.Lcd.clear();
    M5.Lcd.fillScreen(COLOR_BLACK);

    // M5.Speaker.begin();  // スピーカーを使用しないため無効化
    // M5.Imu.begin();      // IMU を使用しないため無効化
    btStop();

    pinMode(9, INPUT_PULLUP);
    pinMode(8, INPUT_PULLUP);
    Wire.begin(9, 8);

    if (!adsConverter.begin()) {
        Serial.println("[ADS1015] init failed… all analog values will be 0");
    }
    adsConverter.setDataRate(RATE_ADS1015_1600SPS);

    if (SENSOR_AMBIENT_LIGHT_PRESENT) {
        // ALS のゲインと積分時間を設定してから初期化
        Ltr5xx_Init_Basic_Para ltr553Params = LTR5XX_BASE_PARA_CONFIG_DEFAULT;
        ltr553Params.als_gain = LTR5XX_ALS_GAIN_48X;
        ltr553Params.als_integration_time = LTR5XX_ALS_INTEGRATION_TIME_300MS;
        CoreS3.Ltr553.begin(&ltr553Params);
        CoreS3.Ltr553.setAlsMode(LTR5XX_ALS_ACTIVE_MODE);
    }
}

// ────────────────────── loop() ──────────────────────
void loop()
{
    static unsigned long lastAlsSampleTime = 0;
    unsigned long now = millis();

    if (now - lastAlsSampleTime >= ALS_MEASUREMENT_INTERVAL_MS) {
        updateBacklightLevel();
        lastAlsSampleTime = now;
    }

    acquireSensorData();
    updateGauges();

    frameCounterPerSecond++;
    if (now - lastFpsTimestamp >= 1000UL) {
        currentFramesPerSecond = frameCounterPerSecond;
        if (DEBUG_MODE_ENABLED)
            Serial.printf("FPS:%d\n", currentFramesPerSecond);
        frameCounterPerSecond = 0;
        lastFpsTimestamp = now;
    }
}
