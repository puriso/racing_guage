#include "config.h"

#include <M5CoreS3.h>
#include <Wire.h>

#include "modules/display.h"
#include "modules/sensor.h"
#include "modules/backlight.h"

// ── LTR553 初期設定 ──
Ltr5xx_Init_Basic_Para ltr553InitParams = LTR5XX_BASE_PARA_CONFIG_DEFAULT;

// ── FPS 計測用 ──
unsigned long previousFpsTimestamp   = 0;
int           frameCounterPerSecond  = 0;
int           currentFramesPerSecond = 0;

// ────────────────────── setup() ──────────────────────
void setup()
{
    Serial.begin(115200);

    M5.begin();
    CoreS3.begin(M5.config());
    M5.Power.begin();
    // 外部からの給電を利用する場合は 5V ピン出力を無効化
    M5.Power.setExtOutput(false);
    CoreS3.Ltr553.begin(&ltr553InitParams);
    M5.Power.begin();
    M5.Power.setExtOutput(false);

    display.init();
    display.setRotation(3);
    display.setColorDepth(16);
    display.setBrightness(BACKLIGHT_DAY);

    mainCanvas.setColorDepth(16);
    mainCanvas.setTextSize(1);
    mainCanvas.createSprite(LCD_WIDTH, LCD_HEIGHT);

    M5.Lcd.clear();
    M5.Lcd.fillScreen(COLOR_BLACK);

    M5.Speaker.begin();
    M5.Imu.begin();
    btStop();

    pinMode(9, INPUT_PULLUP);
    pinMode(8, INPUT_PULLUP);
    Wire.begin(9, 8);

    if (!adsConverter.begin()) {
        Serial.println("[ADS1015] init failed… all analog values will be 0");
    }
    adsConverter.setDataRate(RATE_ADS1015_1600SPS);

    if (SENSOR_AMBIENT_LIGHT_PRESENT) {
        CoreS3.Ltr553.begin(&ltr553InitParams);
        CoreS3.Ltr553.setAlsMode(LTR5XX_ALS_ACTIVE_MODE);
        ltr553InitParams.als_gain             = LTR5XX_ALS_GAIN_48X;
        ltr553InitParams.als_integration_time = LTR5XX_ALS_INTEGRATION_TIME_300MS;
    }
}

// ────────────────────── loop() ──────────────────────
void loop()
{
    static unsigned long previousAlsSampleTime = 0;
    unsigned long now = millis();

    if (now - previousAlsSampleTime >= ALS_MEASUREMENT_INTERVAL_MS) {
        updateBacklightLevel();
        previousAlsSampleTime = now;
    }

    acquireSensorData();
    updateGauges();

    frameCounterPerSecond++;
    if (now - previousFpsTimestamp >= 1000UL) {
        currentFramesPerSecond = frameCounterPerSecond;
        if (DEBUG_MODE_ENABLED)
            Serial.printf("FPS:%d\n", currentFramesPerSecond);
        frameCounterPerSecond = 0;
        previousFpsTimestamp  = now;
    }
}

