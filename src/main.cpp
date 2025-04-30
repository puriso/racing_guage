const bool IS_DEBUG = true;  // デバッグモード

#include <Adafruit_ADS1X15.h>
#include <M5CoreS3.h>
#include <M5GFX.h>
#include <Wire.h>

#include <algorithm>
#include <cmath>  // log関数を使用

#include "LuxManager.h"    // 照度によって画面の明るさを調整
#include "DrawFillArcMeter.h"
// #include "ShiftLamp.h"

// 色設定 (18ビットカラー)
const uint32_t COLOR_WHITE = M5.Lcd.color888(255, 255, 255); // 白
const uint32_t COLOR_BLACK = M5.Lcd.color888(0, 0, 0);      // 黒
const uint32_t COLOR_ORANGE = M5.Lcd.color888(255, 165, 0); // オレンジ
const uint32_t COLOR_YELLOW = M5.Lcd.color888(255, 255, 0);    // 黄
const uint32_t COLOR_RED = M5.Lcd.color888(255, 0, 0);      // 赤

// LCDの幅と高さを定数として定義
const int LCD_WIDTH = 320;   // M5Stack CoreS3のLCD幅
const int LCD_HEIGHT = 240;  // M5Stack CoreS3のLCD高さ

M5GFX display;
M5Canvas canvas(&display);
LuxManager luxManager;

const int GRAPH_WIDTH = 320;
const int GRAPH_HEIGHT = 40;

const uint32_t MAIN_BACKGROUND_COLOR = 0x18E3;

Adafruit_ADS1015 ads;
M5Canvas oilPressCanvas(&canvas);
M5Canvas waterTempCanvas(&canvas);

const float SUPPLY_VOLTAGE = 5.0;           // 基準電圧 (5V)
const unsigned long UPDATE_INTERVAL = 20;  // 更新間隔

const int MAX_PRESSURE_SAMPLES = 1;  // 油圧のサンプル数
const int MAX_TEMP_SAMPLES = 10;     // 水温のサンプル数

const float R25 = 10000.0;                 // 25℃でのサーミスタの抵抗値 (10kΩ)
const float B_CONSTANT = 3380.0;           // サーミスタのB定数
const float T25 = 298.15;                  // 25℃の絶対温度 (K)
const float REFERENCE_RESISTOR = 10000.0;  // 分圧回路の基準抵抗値 (10kΩ)

float pressureValues[MAX_PRESSURE_SAMPLES] = {0};
float maxPressureValue = 0;
float tempValues[MAX_TEMP_SAMPLES] = {0};
float maxTempValue = 0;
bool isPressureOverThreshold = false;
bool isTempOverThreshold = false;
int pressureIndex = 0;
int tempIndex = 0;

float graphData[GRAPH_WIDTH] = {0};  // グラフのデータを保持する配列

Ltr5xx_Init_Basic_Para device_init_base_para = LTR5XX_BASE_PARA_CONFIG_DEFAULT;

void drawOilTempTopBar(M5Canvas &canvas, int OilTempTop, int maxOilTempTop)
{
  const int MAX_DISPLAY_VALUE = 130;
  const int MIN_DISPLAY_VALUE = 80;
  const float MAX_MIN_DIFF = MAX_DISPLAY_VALUE - MIN_DISPLAY_VALUE;
  const int ALERT_THRESHOLD = 120;

  // 背景クリア
  canvas.fillSprite(COLOR_BLACK);

  // バー位置・サイズ
  const int barX = 20;     // バーの左端X座標
  const int barY = 15;     // バーの上端Y座標
  const int barW = 210;    // バーの幅
  const int barH = 20;     // バーの高さ

  // バー背景
  canvas.fillRect(barX + 1, barY + 1, barW - 2, barH - 2, 0x18E3);

  // レッドゾーンの赤帯をメモリと数字の下に常時表示
  {
    const int rx = barX + (int)(barW * ((float)(MAX_DISPLAY_VALUE - MIN_DISPLAY_VALUE) / MAX_MIN_DIFF));  // リミットの位置
    const int ry = barY - 5;                                               // メモリと数字の下に配置
    const int rw = barW - (rx - barX);                                     // 赤帯の幅
    const int rh = 5;                                                      // 赤帯の高さ

    canvas.fillRect(rx, ry, rw, rh, COLOR_RED);  // 赤帯の描画
  }

  // 外枠を白で描画
  //canvas.drawRect(barX, barY, barW, barH, COLOR_WHITE);

  // OilTempTopに応じたバーの塗り分け
  uint32_t color = COLOR_WHITE;
  if (OilTempTop >= ALERT_THRESHOLD)
  {
    color = COLOR_RED;
  }

  int w = (int)((float)barW * ((float)(OilTempTop - MIN_DISPLAY_VALUE) / MAX_MIN_DIFF));  // バーの幅
  // 80度以上のみバーを表示する
  if (OilTempTop >= MIN_DISPLAY_VALUE) {
    canvas.fillRect(barX, barY, w, barH, color);
  }

  // メモリ
  canvas.setTextColor(COLOR_WHITE);
  const int memValues[] = { 80, 90, 100, 110, 120, 130 };
  const int memCount = sizeof(memValues) / sizeof(memValues[0]);

  for (int i = 0; i < memCount; i++)
  {
    float ratio = (float)(memValues[i] - MIN_DISPLAY_VALUE) / MAX_MIN_DIFF;  // メモリの相対位置
    int tickX   = barX + (int)(barW * ratio);              // メモリの位置
    int tickY   = barY - 2;                                // メモリ位置 (バー上)

    // メモリの点線
    canvas.drawPixel(tickX, tickY, COLOR_WHITE);

    // メモリの数字
    canvas.setCursor(tickX - 10, tickY - 12);
    canvas.printf("%d", memValues[i]);

    // 9000OilTempTopのバー内区切り線（グレー）
    if (memValues[i] == ALERT_THRESHOLD)
    {
      canvas.drawLine(tickX, barY, tickX, (barY + barH - 2), M5.Lcd.color888(169, 169, 169));  // グレーの線 (RGB: 169, 169, 169)
    }
  }

  // 現在の回転数 (OilTempTop) と最大回転数 (maxOilTempTop) を表示
  const int infoX = barX + barW - 100;
  const int infoY = barY + barH + 10;
  canvas.setTextSize(1);
  canvas.setTextColor(COLOR_WHITE);
  canvas.setCursor(barX, infoY);
  canvas.printf("MAX OIL.T / Celsius: MAX -> %02d", maxOilTempTop);

  // 右側に絶対値を表示
  char OilTempTopStrings[3];
  sprintf(OilTempTopStrings, "%d", OilTempTop);
  canvas.drawRightString(OilTempTopStrings, barW + (GRAPH_WIDTH - barW) - 1 , 5, &FreeSansBold24pt7b);

}

// 電圧計算関数
auto calculateVoltage(int16_t rawADC) -> float
{
  return (rawADC * 6.144) / 2047.0;  // ADS1015の±6.144V設定を使用
}

// 油圧計算
auto calculateOilPressure(float voltage) -> float
{
  return (voltage > 0.5) ? 250 * (voltage - 0.5) / 100.0 : 0.0;  // kPaをMpa換算
}

// サーミスタの電圧から水温を計算
auto calculateWaterTemp(float voltage) -> float
{
  float resistance = REFERENCE_RESISTOR * ((SUPPLY_VOLTAGE / voltage) - 1);
  float tempK = B_CONSTANT / (log(resistance / R25) + B_CONSTANT / T25);
  // nanの場合は200度として扱う
  return isnan(tempK) ? 200 : tempK - 273.15;  // 摂氏に変換
}

// 平均計算関数
auto calculateAverage(const float values[], int size) -> float
{
  float sum = 0;
  for (int i = 0; i < size; i++)
  {
    sum += values[i];
  }
  return sum / size;
}

// 文字間隔を考慮した中央配置用X座標計算
auto calculateCenteredX(int16_t spriteWidth, const char *text, int spacing = 0, M5Canvas &canvas = canvas) -> int16_t
{
  int totalWidth = 0;

  for (int i = 0; text[i] != '\0'; i++)
  {
    totalWidth += canvas.textWidth(String(text[i]));
    if (text[i + 1] != '\0')
    {  // 最後の文字以外は間隔を加算
      totalWidth += spacing;
    }
  }

  return (spriteWidth - totalWidth) / 2;
}

// メインの数値表示用を文字間隔を考慮して中央配置するテキスト描画
void drawMainValue(int spriteWidth, const char *text, int spacing, int y)
{
  int16_t startX = calculateCenteredX(spriteWidth, text, spacing, canvas);  // 修正した中央配置X座標計算
  int16_t cursorX = startX;                                                 // 描画開始位置を設定

  for (int i = 0; text[i] != '\0'; i++)
  {
    canvas.drawChar(text[i], cursorX, y);  // 文字を描画

    if (text[i + 1] != '\0')
    {  // 最後の文字以外は間隔を加算
      cursorX += canvas.textWidth(String(text[i])) + spacing;
    }
  }
}

// 表示とログ更新
void updateDisplayAndLog(float pressureAvg, float waterTempAverage, float oilVoltage, float waterVoltage, int16_t rawOil, int16_t rawWater, int16_t maxOilTempTop) {

  // レブリミット
  int rpm = pressureAvg * 1000;
  if (rpm >= 8900)
  {
    canvas.createSprite(LCD_WIDTH, LCD_HEIGHT);
    canvas.fillSprite(COLOR_RED);
    canvas.pushSprite(0, 0);  // 表示位置 (x, y)
    return;
  } else if (rpm >= 8700)
  {
    canvas.createSprite(LCD_WIDTH, LCD_HEIGHT);
    canvas.fillSprite(COLOR_YELLOW);
    canvas.pushSprite(0, 0);  // 表示位置 (x, y)
    return;
  }

  canvas.fillSprite(MAIN_BACKGROUND_COLOR);

  // 油温
  canvas.createSprite(320, 60);
  canvas.fillSprite(COLOR_BLACK);
  // OilTempTop
  int OilTempTop = (pressureAvg + 5) * 10;
  if (OilTempTop > maxOilTempTop)
  {
    maxOilTempTop = OilTempTop;
  }
  drawOilTempTopBar(canvas, OilTempTop, maxOilTempTop);
  canvas.pushSprite(0, 0); // 表示位置 (x, y)

  // 油圧
  canvas.setTextColor(COLOR_WHITE);
  canvas.setTextSize(2);
  canvas.setFont(&FreeSansBold24pt7b);
  canvas.setCursor(0, 0);
  drawMainValue(LCD_WIDTH, "OIL PRESSURE", 5, 10);  // 中央配置

  // 水温
  canvas.setTextColor(COLOR_WHITE);
  canvas.setTextSize(2);
  canvas.setFont(&FreeSansBold24pt7b);
  canvas.setCursor(160, 0);
  drawMainValue(LCD_WIDTH, "WATER TEMP", 5, 10);  // 中央配置

  // 油圧名表示
  canvas.createSprite(160, 180);
  drawFillArcMeter(canvas, pressureAvg, 0.0, 10.0, 8.0, RED, "BAR", "OIL.P", maxPressureValue, 0.5, true);
  canvas.pushSprite(0, 60);

  // 水温名表示
  canvas.createSprite(160, 180);
  drawFillArcMeter(canvas, waterTempAverage, 50.0, 110, 98, RED, "Celsius", "WATER.T", maxTempValue, 5.0, false);
  canvas.pushSprite(160, 60);

  if (IS_DEBUG)
  {
    Serial.printf("Oil Pressure: %.2f kPa | Water Temp: %.2f C\n", pressureAvg, waterTempAverage);
    Serial.printf("Oil Voltage: %.2f V | Water Voltage: %.2f V\n", oilVoltage, waterVoltage);
    Serial.printf("Raw Oil: %d | Raw Water: %d\n", rawOil, rawWater);
  }
}

void setup()
{
  Serial.begin(115200);

  M5.begin();

  auto cfg = M5.config();
  CoreS3.begin(cfg);

  M5.Lcd.clear();
  M5.Lcd.fillScreen(BLACK);
  display.init();
  display.setRotation(3);
  display.setColorDepth(24);
  Serial.println("start!");
  M5.Speaker.begin();
  M5.Imu.begin();

  // Bluetoothを無効化
  btStop();

  pinMode(9, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  Wire.begin(9, 8);  // SDA: 9, SCL: 8

  if (!ads.begin())
  {
    Serial.println("Failed to initialize ADS1015! Check connections.");
    display.setCursor(8, 50);
    display.setTextFont(2);
    display.setTextSize(1);
    display.setTextColor(RED);
    display.print("Failed to initialize ADS1015! Check connections.");
    while (1)
    {
      ;  // 初期化失敗時は無限ループ
    }
  }
  // initializeShiftLamp(canvas);

  CoreS3.Ltr553.setAlsMode(LTR5XX_ALS_ACTIVE_MODE);
  device_init_base_para.als_gain = LTR5XX_ALS_GAIN_1X;

  luxManager.initializeLuxSamples();   // 照度サンプルの初期化

  M5.Touch.isEnabled();

  canvas.setColorDepth(24);
  canvas.setTextSize(1);
}


bool isBlink = false;
int maxOilTempTop = 0;
void guageMode()
{
  int16_t rawOil = ads.readADC_SingleEnded(1);
  int16_t rawWater = ads.readADC_SingleEnded(0);

  float oilPressureVoltage = calculateVoltage(rawOil);
  float waterTempVoltage = calculateVoltage(rawWater);

  pressureValues[pressureIndex] = calculateOilPressure(oilPressureVoltage);
  tempValues[tempIndex] = calculateWaterTemp(waterTempVoltage);
  tempIndex = (tempIndex + 1) % MAX_TEMP_SAMPLES;

  float pressureAverage = calculateAverage(pressureValues, MAX_PRESSURE_SAMPLES);
  float tempAverage = calculateAverage(tempValues, MAX_TEMP_SAMPLES);

  maxPressureValue = max(maxPressureValue, pressureAverage);  // 最大油圧値を更新
  maxTempValue = max(maxTempValue, tempAverage);              // 最大水温値を更新

  if (!isTempOverThreshold && tempAverage >= 98)
  {
    isTempOverThreshold = true;
    M5.Speaker.setVolume(100);
    M5.Speaker.tone(3000, 2000);
  }

  updateDisplayAndLog(pressureAverage, tempAverage, oilPressureVoltage, waterTempVoltage, rawOil, rawWater, maxOilTempTop);
}

void detailsMode()
{
  // 表示データ構造体
  struct DisplayData {
    const char* label;  // ラベル名
    float value;        // 値
    const char* unit;   // 単位
  };

  // 表示するデータを管理する配列
  DisplayData displayData[] = {
      {"Max Pressure", 8.5, "BAR"},
      {"Current Pressure", 7.2, "BAR"},
      {"Max Temp", 98.0, "C"},
      {"Current Temp", 95.0, "C"},
      {"Max OilTempTop", 9200, ""},
      {"Current OilTempTop", 7800, ""}
  };
  // 配列の要素数を取得
  const int displayDataCount = sizeof(displayData) / sizeof(displayData[0]);

  canvas.createSprite(320, 240);
  // 情報画面をクリア
  canvas.fillSprite(COLOR_BLACK);
  canvas.setTextColor(COLOR_WHITE);
  canvas.setTextSize(1);

  // 各項目の配置設定
  int lineHeight = 30;  // 行の高さ
  int startY = 10;      // 最初の行のY座標
  int startX = 10;      // ラベルのX座標
  int valueX = 200;     // 値のX座標（ラベルから離す距離）

  // 各データを順次描画
  for (int i = 0; i < displayDataCount; i++) {
    int y = startY + i * lineHeight;

    // ラベルを描画
    canvas.setCursor(startX, y);
    canvas.printf("%s:", displayData[i].label);

    // 値と単位を描画
    canvas.setCursor(valueX, y);
    canvas.printf("%.2f %s", displayData[i].value, displayData[i].unit);
  }

  // 操作案内メッセージを描画
  canvas.setCursor(10, startY + displayDataCount * lineHeight + 10);
  canvas.print("Touch to return");

  // 画面を更新
  canvas.pushSprite(0, 0);
}


enum DisplayMode
{
  GUAGES,
  DETAILS,
  ATTACK
};

// 画面モード値
DisplayMode displayMode = GUAGES;
static m5::touch_state_t prevTouchState;

int prev_x = -1;
int prev_y = -1;
void loop()
{
  CoreS3.update();

  auto touched = CoreS3.Touch.getDetail();
  // タッチされた場合
  if (touched.isPressed()) {
    // 次のディスプレイモードへ
    displayMode = static_cast<DisplayMode>((displayMode + 1) % 2);
    Serial.printf("Display Mode: %d\n", displayMode);
    prevTouchState = touched.state;
    delay(100);
  }

  static unsigned long lastSampleTime = 0;
  static unsigned long lastUpdateTime = 0;
  unsigned long currentMillis = millis();

  switch (displayMode) {
    case GUAGES:
      // ゲージ表示
      if (currentMillis - lastUpdateTime >= UPDATE_INTERVAL) {
        guageMode();  // ゲージモードを描画する関数
      }
      break;

    case DETAILS:
      // 詳細表示
      detailsMode();  // 詳細表示を描画する関数
      break;

    //case ATTACK:
    //  // アタック表示
    //  //attackMode();  // アタック表示を描画する関数
    //  break;

    //default:
    //  // 未知のモード
    //  break;
  }

  lastUpdateTime = currentMillis;  // 更新時刻を記録

  // 照度関連
  if (currentMillis - lastSampleTime >= 500)
  {
    luxManager.updateLuxSamples();
    float averageLux = luxManager.calculateAverageLux();
    // 照度によって画面の明るさを調整
    // 引数: 現在の照度, 最小照度, 最大照度, 最小明るさ, 最大明るさ
    // int brightness = map(averageLux, 0, 100, 50, 150);

    // // 滑らかに明るさを変更
    // if (M5.Lcd.getBrightness() != brightness)
    // {
    //   M5.Lcd.setBrightness(brightness);
    // }

    if (IS_DEBUG)
    {
      Serial.printf("-- LuxManager -------------");
      // Serial.printf("Average Lux: %.2f lx | Brightness: %d\n", averageLux, brightness);
      Serial.printf("---------------------------");
    }
    lastSampleTime = currentMillis;
  }
  delay(1);
}
