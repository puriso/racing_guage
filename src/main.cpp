// ────────────────────── 設定 ──────────────────────
const bool DEBUG_MODE_ENABLED = false;

// ── センサー接続可否（false にするとその項目は常に 0 表示） ──
constexpr bool SENSOR_OIL_PRESSURE_PRESENT  = true;
constexpr bool SENSOR_WATER_TEMP_PRESENT    = true;
constexpr bool SENSOR_OIL_TEMP_PRESENT      = false;
constexpr bool SENSOR_AMBIENT_LIGHT_PRESENT = true;

// ── 標準／ライブラリ ──
#include <Arduino.h>
#include <Adafruit_ADS1X15.h>
#include <M5CoreS3.h>
#include <M5GFX.h>
#include <Wire.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <numeric>

#include "DrawFillArcMeter.h"               // 半円メーター描画

// ── 色設定 (18 bit) ──
constexpr uint32_t COLOR_WHITE  = M5.Lcd.color888(255, 255, 255);
constexpr uint32_t COLOR_BLACK  = M5.Lcd.color888(0,   0,   0);
constexpr uint32_t COLOR_ORANGE = M5.Lcd.color888(255, 165, 0);
constexpr uint32_t COLOR_YELLOW = M5.Lcd.color888(255, 255, 0);
constexpr uint32_t COLOR_RED    = M5.Lcd.color888(255,   0, 0);

// ── 画面サイズ ──
constexpr int LCD_WIDTH  = 320;
constexpr int LCD_HEIGHT = 240;

// ── ALS/輝度自動制御 ──
enum class BrightnessMode { Day, Dusk, Night };
BrightnessMode currentBrightnessMode = BrightnessMode::Day;

constexpr uint32_t LUX_THRESHOLD_DAY  = 15;
constexpr uint32_t LUX_THRESHOLD_DUSK = 10;

constexpr uint8_t  BACKLIGHT_DAY   = 255;
constexpr uint8_t  BACKLIGHT_DUSK  = 200;
constexpr uint8_t  BACKLIGHT_NIGHT =  60;

constexpr int MEDIAN_BUFFER_SIZE = 10;
uint32_t luxSampleBuffer[MEDIAN_BUFFER_SIZE] = {};
int      luxSampleIndex  = 0;

// ── M5 & GFX ──
M5GFX    display;
M5Canvas mainCanvas(&display);

// ── ADS1015 ──
Adafruit_ADS1015 adsConverter;

// ── センサリング用バッファ ──
constexpr int PRESSURE_SAMPLE_SIZE     = 3;
constexpr int WATER_TEMP_SAMPLE_SIZE   = 10;
constexpr int OIL_TEMP_SAMPLE_SIZE     = 10;

float oilPressureSamples[PRESSURE_SAMPLE_SIZE]          = {};
float waterTemperatureSamples[WATER_TEMP_SAMPLE_SIZE]   = {};
float oilTemperatureSamples[OIL_TEMP_SAMPLE_SIZE]       = {};

int oilPressureSampleIndex      = 0;
int waterTemperatureSampleIndex = 0;
int oilTemperatureSampleIndex   = 0;

float recordedMaxOilPressure = 0.0f;
float recordedMaxWaterTemp   = 0.0f;
int   recordedMaxOilTempTop  = 0;

// ── 電圧→物理量変換定数 ──
constexpr float SUPPLY_VOLTAGE          = 5.0f;
constexpr float THERMISTOR_R25          = 10000.0f;
constexpr float THERMISTOR_B_CONSTANT   = 3380.0f;
constexpr float ABSOLUTE_TEMPERATURE_25 = 298.15f;
constexpr float SERIES_REFERENCE_RES    = 10000.0f;

// ── LTR-553 初期化パラメータ ──
Ltr5xx_Init_Basic_Para ltr553InitParams = LTR5XX_BASE_PARA_CONFIG_DEFAULT;

// ── FPS 計測 ──
unsigned long previousFpsTimestamp   = 0;
int           frameCounterPerSecond  = 0;
int           currentFramesPerSecond = 0;

// ────────────────────── プロトタイプ ──────────────────────
void drawOilTemperatureTopBar(M5Canvas& canvas, int oilTemp, int maxOilTemp);
void renderDisplayAndLog(float pressureAvg, float waterTempAvg,
                         int16_t oilTemp, int16_t maxOilTemp);
void updateGauges();
void acquireSensorData();

uint32_t measureLuxWithoutBacklight();
void     updateBacklightLevel();

// ────────────────────── ユーティリティ ──────────────────────
inline float convertAdcToVoltage(int16_t rawAdc)
{
  return (rawAdc * 6.144f) / 2047.0f;
}

inline float convertVoltageToOilPressure(float voltage)
{
  return (voltage > 0.5f) ? 2.5f * (voltage - 0.5f) : 0.0f;   // 0.5 V offset, 2.5 bar/V
}

inline float convertVoltageToTemp(float voltage)
{
  // センサー電圧が 0 の場合はゼロ除算を避けるため早期リターン
  if (voltage <= 0.0f) return 200.0f;
  float resistance = SERIES_REFERENCE_RES * ((SUPPLY_VOLTAGE / voltage) - 1.0f);
  float kelvin     = THERMISTOR_B_CONSTANT /
                     (log(resistance / THERMISTOR_R25) + THERMISTOR_B_CONSTANT / ABSOLUTE_TEMPERATURE_25);
  return std::isnan(kelvin) ? 200.0f : kelvin - 273.15f;
}

template <size_t N>
inline float calculateAverage(const float (&values)[N])
{
  float sum = std::accumulate(values, values + N, 0.0f);
  return sum / static_cast<float>(N);
}

// ────────────────────── ★ ADC セトリング付き読み取り ──────────────────────
constexpr uint32_t ADC_SETTLING_US = 50;             // 残留電荷クリア待ち
int16_t readAdcWithSettling(uint8_t ch)
{
  adsConverter.readADC_SingleEnded(ch);              // ダミー変換
  delayMicroseconds(ADC_SETTLING_US);                // セトリング待ち
  return adsConverter.readADC_SingleEnded(ch);       // 本変換
}


// ────────────────────── 画面更新＋ログ ──────────────────────
void renderDisplayAndLog(float pressureAvg, float waterTempAvg,
                         int16_t oilTemp, int16_t maxOilTemp)
{
  mainCanvas.deleteSprite();
  mainCanvas.createSprite(LCD_WIDTH, LCD_HEIGHT);
  mainCanvas.fillSprite(COLOR_BLACK);
  mainCanvas.setTextColor(COLOR_WHITE);

  // 油温バー
  if (oilTemp > maxOilTemp) maxOilTemp = oilTemp;
  drawOilTemperatureTopBar(mainCanvas, oilTemp, maxOilTemp);

  // メーター (油圧／水温)
  drawFillArcMeter(mainCanvas, pressureAvg,  0.0f, 10.0f,  8.0f,
                   RED, "BAR", "OIL.P", recordedMaxOilPressure,
                   0.5f, true,   0,   60);

  drawFillArcMeter(mainCanvas, waterTempAvg, 50.0f,110.0f, 98.0f,
                   RED, "Celsius", "WATER.T", recordedMaxWaterTemp,
                   5.0f, false, 160,  60);

  // FPS (左下)
  if (DEBUG_MODE_ENABLED) {
    mainCanvas.setTextSize(1);
    mainCanvas.setCursor(5, LCD_HEIGHT - 12);
    mainCanvas.printf("FPS:%d", currentFramesPerSecond);
  }

  mainCanvas.pushSprite(0, 0);
}

// ────────────────────── 油温バー描画 ──────────────────────
void drawOilTemperatureTopBar(M5Canvas& canvas, int oilTemp, int maxOilTemp)
{
  constexpr int MIN_TEMP   =  80;
  constexpr int MAX_TEMP   = 130;
  constexpr int ALERT_TEMP = 120;

  constexpr int X = 20, Y = 15, W = 210, H = 20;
  constexpr float RANGE = MAX_TEMP - MIN_TEMP;

  canvas.fillRect(X + 1, Y + 1, W - 2, H - 2, 0x18E3);

  if (oilTemp >= MIN_TEMP) {
    int barWidth = static_cast<int>(W * (oilTemp - MIN_TEMP) / RANGE);
    uint32_t barColor = (oilTemp >= ALERT_TEMP) ? COLOR_RED : COLOR_WHITE;
    canvas.fillRect(X, Y, barWidth, H, barColor);
  }

  const int marks[] = {80, 90, 100, 110, 120, 130};
  canvas.setTextSize(1);
  canvas.setTextColor(COLOR_WHITE);
  canvas.setFont(&fonts::Font0);

  for (int m : marks) {
    int tx = X + static_cast<int>(W * (m - MIN_TEMP) / RANGE);
    canvas.drawPixel(tx, Y - 2, COLOR_WHITE);
    canvas.setCursor(tx - 10, Y - 14);
    canvas.printf("%d", m);
    if (m == ALERT_TEMP)
      canvas.drawLine(tx, Y, tx, Y + H - 2, M5.Lcd.color888(169, 169, 169));
  }

  canvas.setCursor(X, Y + H + 4);
  canvas.printf("OIL.T / Celsius,  MAX:%03d", maxOilTemp);
  char tempStr[6]; sprintf(tempStr, "%d", oilTemp);
  canvas.setFont(&FreeSansBold24pt7b);
  canvas.drawRightString(tempStr, LCD_WIDTH - 1, 2);
}

// ────────────────────── setup() ──────────────────────
void setup()
{
  Serial.begin(115200);

  M5.begin();
  CoreS3.begin(M5.config());
  // 電源管理ICの初期化
  M5.Power.begin();

  display.init();
  display.setRotation(3);
  display.setColorDepth(24);
  display.setBrightness(BACKLIGHT_DAY);

  mainCanvas.setColorDepth(24);
  mainCanvas.setTextSize(1);

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
constexpr uint32_t ALS_MEASUREMENT_INTERVAL_MS = 8000;  // 8 秒

uint32_t measureLuxWithoutBacklight()
{
  uint8_t prevB = display.getBrightness();
  display.setBrightness(0);
  delayMicroseconds(500);
  uint32_t lux = CoreS3.Ltr553.getAlsValue();
  display.setBrightness(prevB);
  return lux;
}

void updateBacklightLevel()
{
  if (!SENSOR_AMBIENT_LIGHT_PRESENT) {
    if (currentBrightnessMode != BrightnessMode::Day) {
      currentBrightnessMode = BrightnessMode::Day;
      display.setBrightness(BACKLIGHT_DAY);
    }
    return;
  }

  uint32_t lux = measureLuxWithoutBacklight();
  if (DEBUG_MODE_ENABLED) Serial.printf("[ALS] lux=%lu\n", lux);

  luxSampleBuffer[luxSampleIndex] = lux;
  luxSampleIndex = (luxSampleIndex + 1) % MEDIAN_BUFFER_SIZE;

  uint32_t sorted[MEDIAN_BUFFER_SIZE];
  memcpy(sorted, luxSampleBuffer, sizeof(sorted));
  std::nth_element(sorted, sorted + MEDIAN_BUFFER_SIZE / 2, sorted + MEDIAN_BUFFER_SIZE);
  uint32_t medianLux = sorted[MEDIAN_BUFFER_SIZE / 2];

  BrightnessMode newMode =
      (medianLux >= LUX_THRESHOLD_DAY)  ? BrightnessMode::Day  :
      (medianLux >= LUX_THRESHOLD_DUSK) ? BrightnessMode::Dusk : BrightnessMode::Night;

  if (newMode != currentBrightnessMode) {
    currentBrightnessMode = newMode;
    uint8_t targetB =
        (newMode == BrightnessMode::Day)  ? BACKLIGHT_DAY  :
        (newMode == BrightnessMode::Dusk) ? BACKLIGHT_DUSK : BACKLIGHT_NIGHT;
    display.setBrightness(targetB);

    if (DEBUG_MODE_ENABLED) {
      const char* s =
          (newMode == BrightnessMode::Day)  ? "DAY"  :
          (newMode == BrightnessMode::Dusk) ? "DUSK" : "NIGHT";
      Serial.printf("[ALS] median=%lu → mode=%s → brightness=%u\n", medianLux, s, targetB);
    }
  }
}

// ────────────────────── センサ取得 ──────────────────────
void acquireSensorData()
{
  // 油圧
  if (SENSOR_OIL_PRESSURE_PRESENT) {
    int16_t raw = readAdcWithSettling(1);                  // CH1: 油圧
    oilPressureSamples[oilPressureSampleIndex] =
        convertVoltageToOilPressure(convertAdcToVoltage(raw));
  } else {
    oilPressureSamples[oilPressureSampleIndex] = 0.0f;
  }
  oilPressureSampleIndex = (oilPressureSampleIndex + 1) % PRESSURE_SAMPLE_SIZE;

  // 水温
  if (SENSOR_WATER_TEMP_PRESENT) {
    int16_t raw = readAdcWithSettling(0);                  // CH0: 水温
    waterTemperatureSamples[waterTemperatureSampleIndex] =
        convertVoltageToTemp(convertAdcToVoltage(raw));
  } else {
    waterTemperatureSamples[waterTemperatureSampleIndex] = 0.0f;
  }
  waterTemperatureSampleIndex = (waterTemperatureSampleIndex + 1) % WATER_TEMP_SAMPLE_SIZE;

  // 油温
  if (SENSOR_OIL_TEMP_PRESENT) {
    int16_t raw = readAdcWithSettling(2);                  // CH2: 油温
    oilTemperatureSamples[oilTemperatureSampleIndex] =
        convertVoltageToTemp(convertAdcToVoltage(raw));
  } else {
    oilTemperatureSamples[oilTemperatureSampleIndex] = 0.0f;
  }
  oilTemperatureSampleIndex = (oilTemperatureSampleIndex + 1) % OIL_TEMP_SAMPLE_SIZE;

  // 最大値更新
  recordedMaxOilPressure =
      std::max(recordedMaxOilPressure, calculateAverage(oilPressureSamples));
  recordedMaxWaterTemp   =
      std::max(recordedMaxWaterTemp,   calculateAverage(waterTemperatureSamples));
}

// ────────────────────── メーター描画 ──────────────────────
void updateGauges()
{
  float pressureAvg  = calculateAverage(oilPressureSamples);
  float waterTempAvg = calculateAverage(waterTemperatureSamples);
  float oilTempAvg   = calculateAverage(oilTemperatureSamples);

  int oilTempDisplay = static_cast<int>(oilTempAvg);
  if (!SENSOR_OIL_TEMP_PRESENT) oilTempDisplay = 0;

  recordedMaxOilTempTop = std::max(recordedMaxOilTempTop, oilTempDisplay);

  renderDisplayAndLog(pressureAvg, waterTempAvg,
                      oilTempDisplay, recordedMaxOilTempTop);
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
    if (DEBUG_MODE_ENABLED) Serial.printf("FPS:%d\n", currentFramesPerSecond);
    frameCounterPerSecond = 0;
    previousFpsTimestamp  = now;
  }
}
