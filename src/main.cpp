#include <Adafruit_ADS1X15.h>
#include <M5CoreS3.h>
#include <M5GFX.h>
#include <Wire.h>

#include <algorithm>
#include <cmath>  // log関数を使用

#include "GraphManager.h"  // 油圧グラフ用に独自実装
#include "LuxManager.h"    // 照度によって画面の明るさを調整
#include "DrawFillArcMeter.h"

const bool IS_DEBUG = false;  // デバッグモード

// LCDの幅と高さを定数として定義
const int LCD_WIDTH = 320;   // M5Stack CoreS3のLCD幅
const int LCD_HEIGHT = 240;  // M5Stack CoreS3のLCD高さ

M5GFX display;
M5Canvas canvas(&display);
LuxManager luxManager;

const int GRAPH_WIDTH = 320;
const int GRAPH_HEIGHT = 40;
GraphManager graphManager(canvas, GRAPH_WIDTH, GRAPH_HEIGHT);

const uint32_t MAIN_BACKGROUND_COLOR = 0x18E3;

Adafruit_ADS1015 ads;
M5Canvas oilPressCanvas(&canvas);
M5Canvas waterTempCanvas(&canvas);

const float SUPPLY_VOLTAGE = 5.0;           // 基準電圧 (5V)
const unsigned long UPDATE_INTERVAL = 100;  // 更新間隔（100ms）

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
void updateDisplayAndLog(float pressureAvg, float waterTempAverage, float oilVoltage, float waterVoltage, int16_t rawOil,
                         int16_t rawWater)
{
  //// 油圧表示
  canvas.createSprite(160, 200);
  drawFillArcMeter(canvas, pressureAvg, 0.0, 10.0, 8.0, RED, "BAR", "OIL.P", maxPressureValue, 0.5, true);
  canvas.pushSprite(0, 0);

  // 水温表示
  canvas.createSprite(160, 200);
  drawFillArcMeter(canvas, waterTempAverage, 50.0, 110, 98, RED, "Celsius", "WATER.T", maxTempValue, 5.0, false);
  canvas.pushSprite(160, 0);

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

  M5.begin();  // M5Stackの初期化
  M5.Lcd.clear();
  M5.Lcd.fillScreen(BLACK);
  display.init();
  display.setRotation(3);
  display.setColorDepth(24);
  Serial.println("start!");
  M5.Speaker.begin();

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

  CoreS3.Ltr553.setAlsMode(LTR5XX_ALS_ACTIVE_MODE);
  device_init_base_para.als_gain = LTR5XX_ALS_GAIN_1X;

  luxManager.initializeLuxSamples();   // 照度サンプルの初期化
  graphManager.initializeGraphData();  // グラフデータの初期化
}

void loop()
{
  static unsigned long lastSampleTime = 0;
  static unsigned long lastUpdateTime = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - lastUpdateTime >= UPDATE_INTERVAL)
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

    updateDisplayAndLog(pressureAverage, tempAverage, oilPressureVoltage, waterTempVoltage, rawOil, rawWater);

    lastUpdateTime = currentMillis;
    graphManager.drawScrollingLineGraph(pressureAverage);
  }

  if (currentMillis - lastSampleTime >= 500)
  {
    luxManager.updateLuxSamples();
    float averageLux = luxManager.calculateAverageLux();
    // 照度によって画面の明るさを調整
    // 引数: 現在の照度, 最小照度, 最大照度, 最小明るさ, 最大明るさ
    int brightness = map(averageLux, 0, 100, 50, 150);

    // 滑らかに明るさを変更
    if (M5.Lcd.getBrightness() != brightness)
    {
      M5.Lcd.setBrightness(brightness);
    }

    if (IS_DEBUG)
    {
      Serial.printf("-- LuxManager -------------");
      Serial.printf("Average Lux: %.2f lx | Brightness: %d\n", averageLux, brightness);
      Serial.printf("---------------------------");
    }
    lastSampleTime = currentMillis;
  }
  delay(1);
}
