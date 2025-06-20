// ────────────────────── 設定 ──────────────────────
// 設定は config.h で管理する
#include "config.h"

// ── 標準／ライブラリ ──
#include <M5CoreS3.h>
#include <Adafruit_ADS1X15.h>
#include <M5GFX.h>
#include <Wire.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <numeric>
#include <limits>

#include "DrawFillArcMeter.h"               // 半円メーター描画
#include "globals.h"
#include "sensor.h"
#include "display.h"
#include "als.h"

// ── ALS/輝度自動制御 ──
BrightnessMode currentBrightnessMode = BrightnessMode::Day;
uint32_t luxSampleBuffer[MEDIAN_BUFFER_SIZE] = {};
int      luxSampleIndex  = 0;

// ── M5 & GFX ──
M5GFX    display;
M5Canvas mainCanvas(&display);

// ── ADS1015 ──
Adafruit_ADS1015 adsConverter;

// ── センサリング用バッファ ──
float oilPressureSamples[PRESSURE_SAMPLE_SIZE]          = {};
float waterTemperatureSamples[WATER_TEMP_SAMPLE_SIZE]   = {};
float oilTemperatureSamples[OIL_TEMP_SAMPLE_SIZE]       = {};

int oilPressureSampleIndex      = 0;
int waterTemperatureSampleIndex = 0;
int oilTemperatureSampleIndex   = 0;


// 三角マーカーの最大値はメーター描画時（平滑化後）に更新する
float recordedMaxOilPressure = 0.0f;
float recordedMaxWaterTemp   = 0.0f;
int   recordedMaxOilTempTop  = 0;

// メーター描画済みかどうかのフラグ
bool  pressureGaugeInitialized = false;
bool  waterGaugeInitialized    = false;

// ── 表示キャッシュ ──
DisplayCache displayCache = {std::numeric_limits<float>::quiet_NaN(),
                  std::numeric_limits<float>::quiet_NaN(),
                  std::numeric_limits<float>::quiet_NaN(),
                  INT16_MIN};
// 初回描画を強制するため NaN と最小値で初期化しておく


// ── LTR-553 初期化パラメータ ──
Ltr5xx_Init_Basic_Para ltr553InitParams = LTR5XX_BASE_PARA_CONFIG_DEFAULT;

// ── FPS 計測 ──
unsigned long previousFpsTimestamp   = 0;
int           frameCounterPerSecond  = 0;
int           currentFramesPerSecond = 0;

// ────────────────────── setup() ──────────────────────
void setup()
{
  Serial.begin(115200);


  M5.begin();
  CoreS3.begin(M5.config());
  // 電源管理ICの初期化
  M5.Power.begin();
  // LTR553 初期化（サンプルでは begin() 呼び出しが推奨されている）
  CoreS3.Ltr553.begin(&ltr553InitParams);
  // 電源管理ICの初期化
  M5.Power.begin();

  display.init();
  display.setRotation(3);
  // 16bitにすると描画バッファ転送量が減りFPS向上
  display.setColorDepth(16);
  display.setBrightness(BACKLIGHT_DAY);

  mainCanvas.setColorDepth(16);
  mainCanvas.setTextSize(1);
  mainCanvas.createSprite(LCD_WIDTH, LCD_HEIGHT);

  M5.Lcd.clear();
  M5.Lcd.fillScreen(COLOR_BLACK);

  M5.Speaker.begin();
  M5.Imu.begin();
  btStop();   // Bluetooth OFF

  pinMode(9, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  Wire.begin(9, 8);

  if (!adsConverter.begin()) {
    Serial.println("[ADS1015] init failed… all analog values will be 0");
  }
  adsConverter.setDataRate(RATE_ADS1015_1600SPS);

  if (SENSOR_AMBIENT_LIGHT_PRESENT) {
    // LTR553 初期化（サンプルでは begin() 呼び出しが推奨されている）
    CoreS3.Ltr553.begin(&ltr553InitParams);
    CoreS3.Ltr553.setAlsMode(LTR5XX_ALS_ACTIVE_MODE);
    ltr553InitParams.als_gain             = LTR5XX_ALS_GAIN_48X;
    ltr553InitParams.als_integration_time = LTR5XX_ALS_INTEGRATION_TIME_300MS;
  }
}

// ────────────────────── ALS / 輝度制御 ──────────────────────


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
    if (DEBUG_MODE_ENABLED) Serial.printf("FPS:%d\n", currentFramesPerSecond);
    frameCounterPerSecond = 0;
    previousFpsTimestamp  = now;
  }
}
