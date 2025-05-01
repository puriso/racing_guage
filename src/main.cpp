// ────────────────────── 設定 ──────────────────────
// デバッグモード切替
const bool IS_DEBUG = true;

// ── 標準／ライブラリ
#include <Adafruit_ADS1X15.h>
#include <M5CoreS3.h>
#include <M5GFX.h>
#include <Wire.h>
#include <algorithm>
#include <cmath>
#include <cstring>            // memcpy

#include "DrawFillArcMeter.h" // 半円メーター描画
// #include "ShiftLamp.h"      // 未使用なら外してもOK

// 色設定 (18bitカラー)
const uint32_t COLOR_WHITE  = M5.Lcd.color888(255,255,255);
const uint32_t COLOR_BLACK  = M5.Lcd.color888(0,0,0);
const uint32_t COLOR_ORANGE = M5.Lcd.color888(255,165,0);
const uint32_t COLOR_YELLOW = M5.Lcd.color888(255,255,0);
const uint32_t COLOR_RED    = M5.Lcd.color888(255,0,0);

// LCDサイズ
const int LCD_WIDTH  = 320;
const int LCD_HEIGHT = 240;

// ── ALS/輝度自動制御 ───────────────────────────────
enum BrightnessMode { DAY, DUSK, NIGHT };
BrightnessMode brightnessMode = DAY;

// lux しきい値 (要環境キャリブレーション)
constexpr uint32_t LUX_DAY  = 15;   // これ以上→昼
constexpr uint32_t LUX_DUSK = 10;    // これ未満→夜

// バックライト値 (0-255)
constexpr uint8_t B_DAY   = 255;
constexpr uint8_t B_DUSK  = 200;
constexpr uint8_t B_NIGHT = 60;

// 輝度: 中央値用バッファ
constexpr int MEDIAN_BUF = 10;       // 約1分 (60 フレーム/秒想定)
uint32_t luxBuf[MEDIAN_BUF] = {0};
int      luxIdx             = 0;

// ── M5 & GFX オブジェクト
M5GFX    display;
M5Canvas canvas(&display);

// ── ADS1015
Adafruit_ADS1015 ads;

// メーター用キャンバス（必要なら）
M5Canvas oilPressCanvas(&canvas);
M5Canvas waterTempCanvas(&canvas);

// センササンプルバッファ
const int MAX_PRESSURE_SAMPLES = 1;
const int MAX_TEMP_SAMPLES     = 10;
float pressureValues[MAX_PRESSURE_SAMPLES] = {0};
float tempValues    [MAX_TEMP_SAMPLES]     = {0};
int   pressureIndex = 0;
int   tempIndex     = 0;

// 最大値トラッキング
float maxPressureValue = 0.0f;
float maxTempValue     = 0.0f;
int   maxOilTempTop    = 0;

// 電圧変換パラメータ
const float SUPPLY_VOLTAGE     = 5.0f;
const float R25                = 10000.0f;
const float B_CONSTANT         = 3380.0f;
const float T25                = 298.15f;
const float REFERENCE_RESISTOR = 10000.0f;

// ALS 初期化パラメータ
Ltr5xx_Init_Basic_Para device_init_base_para = LTR5XX_BASE_PARA_CONFIG_DEFAULT;

// FPS 用カウンタ
unsigned long fpsLastTime   = 0;
int           fpsFrameCount = 0;
int           currentFps    = 0;

// 表示モード（未使用なら削除可）
enum DisplayMode { gaugeS, DETAILS, ATTACK };
DisplayMode displayMode = gaugeS;
static m5::touch_state_t prevTouchState;

// ────────────────────── プロトタイプ ──────────────────────
void drawOilTempTopBar(M5Canvas &canvas, int oilTemp, int maxOilTemp);
void updateDisplayAndLog(float pressureAvg, float waterTempAvg,
                         int16_t oilTemp, int16_t maxOilTemp, int16_t rpm);
void pushGaugeSprite();
void fetchSensorData();

// ALS/輝度制御
uint32_t sampleLuxWithoutBacklight();
void adjustBrightness();

// ────────────────────── ユーティリティ ──────────────────────
inline float calculateVoltage(int16_t rawADC) {
  return (rawADC * 6.144f) / 2047.0f;
}

inline float calculateOilPressure(float voltage) {
  return (voltage > 0.5f)
    ? 250.0f * (voltage - 0.5f) / 100.0f
    : 0.0f;
}

inline float calculateWaterTemp(float voltage) {
  float r  = REFERENCE_RESISTOR * ((SUPPLY_VOLTAGE / voltage) - 1.0f);
  float tK = B_CONSTANT / (log(r / R25) + B_CONSTANT / T25);
  return std::isnan(tK) ? 200.0f : tK - 273.15f;
}

inline float calculateAverage(const float vals[], int n) {
  float sum = 0;
  for (int i = 0; i < n; ++i) sum += vals[i];
  return sum / n;
}

void drawRPMValue(int rpm, int x, int y, const lgfx::IFont* font) {
  char buf[10];
  sprintf(buf, "%d", rpm);
  canvas.drawCenterString(buf, x, y, font);
}

// ────────────────────── 画面更新＋ログ ──────────────────────
void updateDisplayAndLog(float pressureAvg, float waterTempAvg,
                         int16_t oilTemp, int16_t maxOilTemp, int16_t rpm)
{
  canvas.deleteSprite();
  canvas.clear();

  // レブリミット (例: 8850 以上で赤)
  if (rpm >= 8850) {
    canvas.createSprite(LCD_WIDTH, LCD_HEIGHT);
    canvas.fillSprite(COLOR_RED);
    canvas.setTextColor(COLOR_WHITE);
    drawRPMValue(rpm * 0.01, LCD_WIDTH/2, 40, &fonts::Font8);
    canvas.setFont(&fonts::Orbitron_Light_24);
    canvas.drawCenterString("RPM / x100", LCD_WIDTH/2, 125);
    canvas.pushSprite(0,0);
    return;
  }
  else if (rpm >= 8400) {
    canvas.createSprite(LCD_WIDTH, LCD_HEIGHT);
    canvas.fillSprite(COLOR_YELLOW);
    canvas.setTextColor(COLOR_BLACK);
    drawRPMValue(rpm * 0.01, LCD_WIDTH/2, 40, &fonts::Font8);
    canvas.setFont(&fonts::Orbitron_Light_24);
    canvas.drawCenterString("RPM / x100", LCD_WIDTH/2, 125);
    canvas.pushSprite(0,0);
    return;
  }

  // 通常描画
  canvas.createSprite(LCD_WIDTH, LCD_HEIGHT);
  canvas.fillSprite(COLOR_BLACK);
  canvas.setTextColor(COLOR_WHITE);

  // 油温バー
  if (oilTemp > maxOilTemp) maxOilTemp = oilTemp;
  drawOilTempTopBar(canvas, oilTemp, maxOilTemp);

  // 油圧メーター
  drawFillArcMeter(canvas, pressureAvg,  0.0f, 10.0f,  8.0f,
                   RED, "BAR", "OIL.P", maxPressureValue,
                   0.5f, true,   0, 60);
  // 水温メーター
  drawFillArcMeter(canvas, waterTempAvg, 50.0f,110.0f, 98.0f,
                   RED, "Celsius", "WATER.T", maxTempValue,
                   5.0f, false, 160, 60);

  // RPM表示
  drawRPMValue(rpm, LCD_WIDTH/2, 70, &fonts::Font0);
  canvas.drawCenterString("RPM", LCD_WIDTH/2, 80);

  // FPS表示 (左下)
  if(IS_DEBUG) {
    canvas.setTextSize(1);
    canvas.setCursor(5, LCD_HEIGHT - 12);
    canvas.printf("FPS:%d", currentFps);
  }

  canvas.pushSprite(0,0);
}

// ────────────────────── 油温トップバー ──────────────────────
void drawOilTempTopBar(M5Canvas &canvas, int oilTemp, int maxOilTemp)
{
  const int MIN_V = 80, MAX_V = 130, ALERT_V = 120;
  const float RANGE = MAX_V - MIN_V;
  const int X = 20, Y = 15, W = 210, H = 20;

  // 背景
  canvas.fillRect(X+1,Y+1,W-2,H-2, 0x18E3);

  // バー
  if (oilTemp >= MIN_V) {
    int w = (W * (oilTemp - MIN_V) / RANGE);
    canvas.fillRect(X, Y, w, H, (oilTemp >= ALERT_V ? COLOR_RED : COLOR_WHITE));
  }

  // メモリ＆数字
  int marks[] = {80,90,100,110,120,130};
  canvas.setTextSize(1);
  canvas.setTextColor(COLOR_WHITE);
  canvas.setFont(&fonts::Font0);
  for (int v : marks) {
    int tx = X + (W * (v - MIN_V) / RANGE);
    canvas.drawPixel(tx, Y-2, COLOR_WHITE);
    canvas.setCursor(tx-10, Y-14);
    canvas.printf("%d", v);
    if (v == ALERT_V)
      canvas.drawLine(tx, Y, tx, Y + H - 2, M5.Lcd.color888(169,169,169));
  }

  // 数値表示
  canvas.setCursor(X, Y + H + 10);
  canvas.printf("OIL.T/ Celsius, MAX:%03d", maxOilTemp);
  char buf[5]; sprintf(buf, "%d", oilTemp);
  canvas.setFont(&FreeSansBold24pt7b);
  canvas.drawRightString(buf, LCD_WIDTH - 1, 5);
}

// ────────────────────── setup() ──────────────────────
void setup()
{
  Serial.begin(115200);

  M5.begin();
  auto cfg = M5.config();
  CoreS3.begin(cfg);

  display.init();
  display.setRotation(3);
  display.setColorDepth(24);
  display.setBrightness(B_DAY);  // 初期輝度

  canvas.setColorDepth(24);
  canvas.setTextSize(1);

  M5.Lcd.clear();
  M5.Lcd.fillScreen(COLOR_BLACK);

  M5.Speaker.begin();
  M5.Imu.begin();
  btStop();                     // Bluetooth off

  pinMode(9, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  Wire.begin(9,8);

  if (!ads.begin()) {
    Serial.println("ADS1015 init failed");
    display.setCursor(8,50);
    display.setTextColor(COLOR_RED);
    display.print("ADS1015 init failed!");
    while(true);
  }

  // LTR-553 ALS 起動
  CoreS3.Ltr553.setAlsMode(LTR5XX_ALS_ACTIVE_MODE);
  device_init_base_para.als_gain = LTR5XX_ALS_GAIN_48X;
  device_init_base_para.als_integration_time = LTR5XX_ALS_INTEGRATION_TIME_300MS;
}

// ────────────────────── ALS / 輝度制御 ──────────────────────
// ALS 取得間隔 (フリッカー対策) ---------------
constexpr uint32_t N_SECONDS_BETWEEN_ALS = 10; // ← ここを書き換えで自由に設定
uint32_t sampleLuxWithoutBacklight()
{
  uint8_t oldB = display.getBrightness();
  display.setBrightness(0);           // 一瞬消灯
  delayMicroseconds(500);            // N ms
  /* LTR-553 の ALS 取得は getAlsValue() が正しい  :contentReference[oaicite:0]{index=0} */
  uint16_t lux16 = CoreS3.Ltr553.getAlsValue();
  display.setBrightness(oldB);               // 復帰

  return (uint32_t)lux16;                    // uint32_t に拡張して返す
}

void adjustBrightness()
{
  // ① lux 取得
  uint32_t lux = sampleLuxWithoutBacklight();
  Serial.printf("[ALS] lux=%lu\n", lux);

  // ② バッファ更新
  luxBuf[luxIdx] = lux;
  luxIdx = (luxIdx + 1) % MEDIAN_BUF;

  // ③ 中央値
  uint32_t tmp[MEDIAN_BUF];
  memcpy(tmp, luxBuf, sizeof(tmp));
  std::nth_element(tmp, tmp + MEDIAN_BUF/2, tmp + MEDIAN_BUF);
  uint32_t medianLux = tmp[MEDIAN_BUF/2];

  // ④ モード判定
  BrightnessMode newMode =
      (medianLux >= LUX_DAY)  ? DAY  :
      (medianLux >= LUX_DUSK) ? DUSK : NIGHT;

  // ⑤ 変化時のみ反映
  if (newMode != brightnessMode) {
    brightnessMode = newMode;
    uint8_t target =
        (brightnessMode == DAY)   ? B_DAY   :
        (brightnessMode == DUSK)  ? B_DUSK  : B_NIGHT;
    display.setBrightness(target);
    if (IS_DEBUG) {
      Serial.printf("[ALS] median=%lu lux → mode=%s → brightness=%u\n",
                    medianLux,
                    (brightnessMode==DAY ? "DAY" :
                     brightnessMode==DUSK? "DUSK":"NIGHT"),
                    target);
    }
  }
}

// ────────────────────── センサ取得 ──────────────────────
void fetchSensorData()
{
  int16_t rawOil   = ads.readADC_SingleEnded(1);
  int16_t rawWater = ads.readADC_SingleEnded(0);

  float oilVol   = calculateVoltage(rawOil);
  float waterVol = calculateVoltage(rawWater);

  pressureValues[pressureIndex] = calculateOilPressure(oilVol);
  tempValues    [tempIndex]     = calculateWaterTemp(waterVol);
  pressureIndex = (pressureIndex + 1) % MAX_PRESSURE_SAMPLES;
  tempIndex     = (tempIndex     + 1) % MAX_TEMP_SAMPLES;

  maxPressureValue = std::max(maxPressureValue,
                              calculateAverage(pressureValues,
                                               MAX_PRESSURE_SAMPLES));
  maxTempValue     = std::max(maxTempValue,
                              calculateAverage(tempValues,
                                               MAX_TEMP_SAMPLES));
}

// ────────────────────── メーター描画 ──────────────────────
void pushGaugeSprite()
{
  float pressureAvg = calculateAverage(pressureValues, MAX_PRESSURE_SAMPLES);
  float tempAvg     = calculateAverage(tempValues,     MAX_TEMP_SAMPLES);

  // ダミー: 任意ロジックで oilTemp/rpm を決定
  int oilTemp = static_cast<int>((pressureAvg + 5) * 10);
  int rpm     = static_cast<int>(pressureAvg * 1000);

  updateDisplayAndLog(pressureAvg, tempAvg,
                      oilTemp, maxOilTempTop,
                      rpm);
}

// ────────────────────── loop() ──────────────────────
static unsigned long lastAlsSample = 0;   // 前回 ALS を取った時刻
constexpr uint32_t   ALS_INTERVAL_MS = 8000;  // 8 秒
void loop()
{
  static unsigned long lastAlsSample = 0;
  unsigned long now = millis();

  // ── N 秒ごとに 1 回だけ輝度制御 ───────────────────
  if (now - lastAlsSample >= ALS_INTERVAL_MS) {   // 8 秒以上経過した？
    adjustBrightness();                 // 明るさ自動調整 (ALS 取得)
    lastAlsSample = now;                // タイムスタンプ更新
  }

  pushGaugeSprite();    // 画面更新
  fetchSensorData();    // センサ取得

  // FPS 計測
  static uint32_t fpsFrameCount = 0;
  fpsFrameCount++;
  if (now - fpsLastTime >= 1000UL) {
    currentFps  = fpsFrameCount;
    if(IS_DEBUG) Serial.printf("FPS:%d\n", currentFps);
    fpsFrameCount = 0;
    fpsLastTime   = now;
  }
}
