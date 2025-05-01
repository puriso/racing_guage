// デバッグモード切替
const bool IS_DEBUG = false;

#include <Adafruit_ADS1X15.h>
#include <M5CoreS3.h>
#include <M5GFX.h>
#include <Wire.h>
#include <algorithm>
#include <cmath>  // log関数

#include "LuxManager.h"        // 照度によるバックライト調整
#include "DrawFillArcMeter.h"  // 半円メーター描画
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

// M5 & GFX オブジェクト
M5GFX    display;
M5Canvas canvas(&display);
LuxManager luxManager;

// グラフ用（未使用なら削除可）
const int GRAPH_WIDTH  = 320;
const int GRAPH_HEIGHT = 40;
float graphData[GRAPH_WIDTH] = {0};

// ADS1015
Adafruit_ADS1015 ads;
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
const float SUPPLY_VOLTAGE    = 5.0f;
const float R25               = 10000.0f;
const float B_CONSTANT        = 3380.0f;
const float T25               = 298.15f;
const float REFERENCE_RESISTOR = 10000.0f;

// ALS 初期化パラメータ
Ltr5xx_Init_Basic_Para device_init_base_para = LTR5XX_BASE_PARA_CONFIG_DEFAULT;

// FPS 用カウンタ
unsigned long fpsLastTime   = 0;
int          fpsFrameCount  = 0;
int          currentFps     = 0;

// 表示モード
enum DisplayMode { gaugeS, DETAILS, ATTACK };
DisplayMode displayMode = gaugeS;
static m5::touch_state_t prevTouchState;

// プロトタイプ宣言
void drawOilTempTopBar(M5Canvas &canvas, int oilTemp, int maxOilTemp);
void updateDisplayAndLog(float pressureAvg, float waterTempAvg,
                         float oilVol, float waterVol,
                         int16_t rawOil, int16_t rawWater,
                         int16_t oilTemp, int16_t maxOilTemp, int16_t rpm);
void gaugeMode();
void detailsMode();

// ————————————————————
// 電圧→電圧(V) 変換
inline float calculateVoltage(int16_t rawADC) {
  return (rawADC * 6.144f) / 2047.0f;
}

// 電圧→油圧(bar) 変換（元実装に合わせて 250*(V-0.5)/100）
inline float calculateOilPressure(float voltage) {
  return (voltage > 0.5f)
    ? 250.0f * (voltage - 0.5f) / 100.0f
    : 0.0f;
}

// 電圧→水温(℃) 変換
inline float calculateWaterTemp(float voltage) {
  float r = REFERENCE_RESISTOR * ((SUPPLY_VOLTAGE / voltage) - 1.0f);
  float tK = B_CONSTANT / (log(r / R25) + B_CONSTANT / T25);
  return std::isnan(tK) ? 200.0f : tK - 273.15f;
}

// 平均計算
inline float calculateAverage(const float vals[], int n) {
  float sum = 0;
  for (int i = 0; i < n; ++i) sum += vals[i];
  return sum / n;
}

// RPM 文字列描画
void drawRPMValue(int rpm, int x, int y, const lgfx::IFont* font) {
  char buf[10];
  sprintf(buf, "%d", rpm);
  canvas.drawCenterString(buf, x, y, font);
}

// ————————————————————
// 画面更新＋ログ（FPS表示・シリアル出力含む）
// ————————————————————
void updateDisplayAndLog(float pressureAvg, float waterTempAvg,
                         float oilVol, float waterVol,
                         int16_t rawOil, int16_t rawWater,
                         int16_t oilTemp, int16_t maxOilTemp, int16_t rpm)
{
  canvas.deleteSprite();
  canvas.createSprite(LCD_WIDTH, LCD_HEIGHT);

  // レブリミット（元の早期returnロジック）
  if (rpm >= 8900) {
    canvas.fillSprite(RED);
    canvas.setTextColor(COLOR_WHITE);
    canvas.setTextSize(2);
    drawRPMValue(rpm, LCD_WIDTH/2, 20, &fonts::Font7);
    canvas.pushSprite(0,0);
    return;
  }
  else if (rpm >= 8400) {
    canvas.fillSprite(YELLOW);
    canvas.setTextColor(COLOR_BLACK);
    canvas.setTextSize(2);
    drawRPMValue(rpm, LCD_WIDTH/2, 20, &fonts::Font7);
    canvas.pushSprite(0,0);
    return;
  }

  // 通常描画
  canvas.fillSprite(COLOR_BLACK);
  canvas.setTextColor(COLOR_WHITE);
  canvas.setTextSize(0);

  // 油温バー
  if (oilTemp > maxOilTemp) maxOilTemp = oilTemp;
  drawOilTempTopBar(canvas, oilTemp, maxOilTemp);

  // 油圧メーター
  drawFillArcMeter(canvas, pressureAvg,  0.0f, 10.0f,  8.0f,
                   RED, "BAR", "OIL.P", maxPressureValue,
                   0.5f, true,  0, 60);
  // 水温メーター
  drawFillArcMeter(canvas, waterTempAvg, 50.0f,110.0f, 98.0f,
                   RED, "Celsius",   "WATER.T", maxTempValue,
                   5.0f, false,160,60);

  // RPM表示
  drawRPMValue(rpm, LCD_WIDTH/2, 60, &fonts::Font0);
  canvas.drawCenterString("RPM", LCD_WIDTH/2, 70);

  // FPS表示（左下）
  canvas.setTextSize(1);
  canvas.setCursor(5, LCD_HEIGHT - 12);
  canvas.printf("FPS:%d", currentFps);

  canvas.pushSprite(0,0);

  // シリアル出力（デバッグ）
  if (IS_DEBUG) {
    Serial.printf("OilP:%.2fbar WatT:%.2fC V(oil):%.2f V(wat):%.2f FPS:%d\n",
                  pressureAvg, waterTempAvg, oilVol, waterVol, currentFps);
  }
}

// ————————————————————
// 油温トップバー描画
// ————————————————————
void drawOilTempTopBar(M5Canvas &canvas, int oilTemp, int maxOilTemp)
{
  const int MIN_V = 80, MAX_V = 130, ALERT_V = 120;
  const float RANGE = MAX_V - MIN_V;
  const int X = 20, Y = 15, W = 210, H = 20;

  // 背景
  canvas.fillRect(X+1,Y+1,W-2,H-2, 0x18E3);

  // 条件付きバー
  if (oilTemp >= MIN_V) {
    int w = (W*(oilTemp - MIN_V)/RANGE);
    canvas.fillRect(X,Y,w,H, (oilTemp>=ALERT_V?COLOR_RED:COLOR_WHITE));
  }

  // メモリ＆数字
  int marks[] = {80,90,100,110,120,130};
  canvas.setTextSize(1);
  canvas.setTextColor(COLOR_WHITE);
  canvas.setFont(&fonts::Font0);
  for (int v:marks) {
    int tx = X + (W*(v - MIN_V)/RANGE);
    canvas.drawPixel(tx, Y-2, COLOR_WHITE);
    canvas.setCursor(tx-10, Y-14);
    canvas.printf("%d", v);
    if (v==ALERT_V) canvas.drawLine(tx,Y,tx,Y+H-2, M5.Lcd.color888(169,169,169));
  }

  // 絶対値
  canvas.setCursor(X, Y+H+10);
  canvas.printf("OIL.T/ Celsius, MAX: %03d℃", maxOilTemp);
  char buf[4]; sprintf(buf,"%d", oilTemp);
  canvas.drawRightString(buf, LCD_WIDTH - 1, 5, &FreeSansBold24pt7b);
}

// ————————————————————
// setup()
// ————————————————————
void setup()
{
  Serial.begin(115200);

  M5.begin();
  auto cfg = M5.config();
  CoreS3.begin(cfg);

  M5.Lcd.clear();
  M5.Lcd.fillScreen(COLOR_BLACK);

  display.init();
  display.setRotation(3);
  display.setColorDepth(24);

  canvas.setColorDepth(24);
  canvas.setTextSize(1);

  M5.Speaker.begin();
  M5.Imu.begin();
  btStop();  // Bluetooth off

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

  CoreS3.Ltr553.setAlsMode(LTR5XX_ALS_ACTIVE_MODE);
  device_init_base_para.als_gain = LTR5XX_ALS_GAIN_1X;
  luxManager.initializeLuxSamples();
}

// ————————————————————
// ゲージモード描画
// ————————————————————
void gaugeMode()
{
  int16_t rawOil   = ads.readADC_SingleEnded(1);
  int16_t rawWater = ads.readADC_SingleEnded(0);

  float oilVol   = calculateVoltage(rawOil);
  float waterVol = calculateVoltage(rawWater);

  pressureValues[pressureIndex] = calculateOilPressure(oilVol);
  tempValues    [tempIndex]     = calculateWaterTemp(waterVol);
  pressureIndex = (pressureIndex + 1) % MAX_PRESSURE_SAMPLES;
  tempIndex     = (tempIndex     + 1) % MAX_TEMP_SAMPLES;

  float pressureAvg = calculateAverage(pressureValues, MAX_PRESSURE_SAMPLES);
  float tempAvg     = calculateAverage(tempValues,     MAX_TEMP_SAMPLES);

  maxPressureValue = std::max(maxPressureValue, pressureAvg);
  maxTempValue     = std::max(maxTempValue,     tempAvg);

  int oilTemp = static_cast<int>((pressureAvg + 5) * 10);
  int rpm     = static_cast<int>(pressureAvg * 1000);

  updateDisplayAndLog(pressureAvg, tempAvg,
                      oilVol,    waterVol,
                      rawOil,    rawWater,
                      oilTemp,   maxOilTempTop,
                      rpm);
}

// ————————————————————
// 詳細モード（省略）
// ————————————————————
void detailsMode()
{
  // ここに詳細画面描画を入れる
}

// ————————————————————
// loop()
// ————————————————————
void loop()
{
  static unsigned long lastSampleTime = 0;
  unsigned long currentMillis = millis();

  gaugeMode();

  // 照度関連更新
  if (currentMillis - lastSampleTime >= 500) {
    luxManager.updateLuxSamples();
    lastSampleTime = currentMillis;
  }

  // FPS計測＆シリアル出力
  fpsFrameCount++;
  if (currentMillis - fpsLastTime >= 1000) {
    currentFps    = fpsFrameCount;
    fpsFrameCount = 0;
    fpsLastTime   = currentMillis;
    Serial.printf("FPS: %d\n", currentFps);
  }
}
