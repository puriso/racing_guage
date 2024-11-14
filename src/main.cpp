#include <Wire.h>
#include <M5CoreS3.h>
#include <Adafruit_ADS1X15.h>
#include <cmath> // log関数を使用
#include <M5GFX.h>
#include "LuxManager.h" // 照度によって画面の明るさを調整
#include "GraphManager.h" // 油圧グラフ用に独自実装

// M5Stack CoreS3 LCD
const int SPRITE_WIDTH = 320;
const int SPRITE_HEIGHT = 40;

M5GFX display;
M5Canvas canvas(&display);
LuxManager luxManager;
GraphManager graphManager(canvas, SPRITE_WIDTH, SPRITE_HEIGHT);

const uint32_t MAIN_BACKGROUND_COLOR = 0x18E3;

Adafruit_ADS1015 ads;
M5Canvas oilPressCanvas(&canvas);
M5Canvas waterTempCanvas(&canvas);

const float SUPPLY_VOLTAGE = 5.0;       // 基準電圧 (5V)
const unsigned long UPDATE_INTERVAL = 100; // 更新間隔（100ms）

const int MAX_PRESSURE_SAMPLES = 3;     // 油圧のサンプル数
const int MAX_TEMP_SAMPLES = 10;        // 水温のサンプル数

const float R25 = 10000.0;              // 25℃でのサーミスタの抵抗値 (10kΩ)
const float B_CONSTANT = 3380.0;        // サーミスタのB定数
const float T25 = 298.15;               // 25℃の絶対温度 (K)
const float REFERENCE_RESISTOR = 10000.0; // 分圧回路の基準抵抗値 (10kΩ)

float pressureValues[MAX_PRESSURE_SAMPLES] = {0};
float tempValues[MAX_TEMP_SAMPLES] = {0};
int pressureIndex = 0;
int tempIndex = 0;

float graphData[SPRITE_WIDTH] = { 0 };  // グラフのデータを保持する配列

// 電圧計算関数
float calculateVoltage(int16_t rawADC) {
  return (rawADC * 6.144) / 2047.0;  // ADS1015の±6.144V設定を使用
}

// 油圧計算
float calculateOilPressure(float voltage) {
  return (voltage > 0.5) ? 250 * (voltage - 0.5) / 100.0 : 0.0;  // kPaをMpa換算
}

// サーミスタの電圧から水温を計算
float calculateWaterTemp(float voltage) {
  float resistance = REFERENCE_RESISTOR * ((SUPPLY_VOLTAGE / voltage) - 1);
  float tempK = B_CONSTANT / (log(resistance / R25) + B_CONSTANT / T25);
  return tempK - 273.15;  // 摂氏温度
}

// 平均計算関数
float calculateAverage(float values[], int size) {
  float sum = 0;
  for (int i = 0; i < size; i++) {
    sum += values[i];
  }
  return sum / size;
}

// グラフデータ初期化
void initializeGraphData(float graphData[], int width, int height) {
  for (int i = 0; i < width; i++) {
    graphData[i] = height;  // 初期値
  }
}


// テキスト中央配置X座標計算
int16_t calculateCenteredX(int16_t spriteWidth, const char* text, M5Canvas& canvas) {
  return (spriteWidth - canvas.textWidth(text)) / 2;
}

// 表示とログ更新
void updateDisplayAndLog(float pressureAvg, float waterTempAverage, float oilVoltage, float waterVoltage, int16_t rawOil, int16_t rawWater) {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setTextFont(8);

  // 油圧表示
  canvas.createSprite(160, 200);
  uint16_t pressureColor = (pressureAvg >= 8.0) ? RED : MAIN_BACKGROUND_COLOR;
  canvas.fillScreen(pressureColor);
  canvas.setFont(&FreeSansBold24pt7b);
  canvas.setTextColor(WHITE, pressureColor);

  char text[10];
  snprintf(text, sizeof(text), "%.1f", pressureAvg);
  canvas.setCursor(calculateCenteredX(160, text, canvas), 72);
  canvas.printf("%s", text);
  // 油圧の単位を表示
  canvas.setTextFont(&Font0);
  canvas.setCursor(calculateCenteredX(160, "O.PRS / BAR", canvas), 122);
  canvas.printf("O.PRS / BAR");
  canvas.pushSprite(0, 0);
   canvas.setCursor(8, 185);
  canvas.printf("%s", "O.PRS / Graph");

  // 水温表示
  canvas.createSprite(160, 200);
  uint16_t tempColor = (waterTempAverage > 97.9) ? RED : MAIN_BACKGROUND_COLOR;
  canvas.fillScreen(tempColor);
  canvas.setTextColor(WHITE, tempColor);
  snprintf(text, sizeof(text), "%.0f", waterTempAverage);
    char tempText[10];
  snprintf(tempText, sizeof(tempText), "%.0f", waterTempAverage);
  canvas.setCursor(calculateCenteredX(160, tempText, canvas), 72);
  canvas.setFont(&FreeSansBold24pt7b);
  canvas.printf("%s", tempText);
  // 水温の単位を表示
  canvas.setTextFont(&Font0);
  canvas.setCursor(calculateCenteredX(160, "W.TEMP / Celsius", canvas), 122);
  canvas.printf("W.TEMP / Celsius");
  canvas.pushSprite(160, 0);
}

// ヘッダー描画
void renderHeaders() {
  display.setTextColor(WHITE);
  display.setCursor(8, 24);
  display.setTextSize(1);
  display.setTextFont(0);
  display.setTextColor(0x7BEF, BLACK);
}

void setup() {
  M5.begin();  // M5Stackの初期化
  M5.Lcd.fillScreen(BLACK);
  Serial.println("start!");
  pinMode(9, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  Wire.begin(9, 8);  // SDA: 9, SCL: 8
  Serial.begin(115200);

  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS1015! Check connections.");
    M5.Lcd.setCursor(8, 50);
    M5.Lcd.setTextFont(2);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(RED);
    M5.Lcd.print("Failed to initialize ADS1015! Check connections.");
    while (1);  // 初期化失敗時は無限ループ
  }

  display.init();
  display.fillScreen(MAIN_BACKGROUND_COLOR);
  display.setRotation(1);
  display.setColorDepth(24);
  display.setTextFont(8);

  renderHeaders();
  initializeGraphData(graphData, SPRITE_WIDTH, SPRITE_HEIGHT);
}

void loop() {
  static unsigned long lastSampleTime = 0;
  static unsigned long lastUpdateTime = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - lastUpdateTime >= UPDATE_INTERVAL) {
    int16_t rawOil = ads.readADC_SingleEnded(1);
    int16_t rawWater = ads.readADC_SingleEnded(0);

    float oilPressureVoltage = calculateVoltage(rawOil);
    float waterTempVoltage = calculateVoltage(rawWater);

    pressureValues[pressureIndex] = calculateOilPressure(oilPressureVoltage);
    tempValues[tempIndex] = calculateWaterTemp(waterTempVoltage);
    pressureIndex = (pressureIndex + 1) % MAX_PRESSURE_SAMPLES;
    tempIndex = (tempIndex + 1) % MAX_TEMP_SAMPLES;

    float pressureAverage = calculateAverage(pressureValues, MAX_PRESSURE_SAMPLES);
    float tempAverage = calculateAverage(tempValues, MAX_TEMP_SAMPLES);

    updateDisplayAndLog(pressureAverage, tempAverage, oilPressureVoltage, waterTempVoltage, rawOil, rawWater);

    lastUpdateTime = currentMillis;
    graphManager.drawScrollingLineGraph(pressureAverage);
  }

  if (currentMillis - lastSampleTime >= UPDATE_INTERVAL) {
    luxManager.updateLuxSamples();
    float averageLux = luxManager.calculateAverageLux();
    int brightness = map(averageLux, 50, 250, 80, 255);
    brightness = constrain(brightness, 10, 255);
    M5.Lcd.setBrightness(brightness >= 70 ? brightness : 70);

    Serial.printf("Average Lux: %.2f lx | Brightness: %d\n", averageLux, brightness);
    lastSampleTime = currentMillis;
  }
  delay(1);
}
