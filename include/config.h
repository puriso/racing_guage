#ifndef CONFIG_H
#define CONFIG_H

#include <M5CoreS3.h>

// ────────────────────── 設定 ──────────────────────
// デバッグモードを有効にするかどうか
constexpr bool DEBUG_MODE_ENABLED = false;

// ── センサー接続可否（false にするとその項目は常に 0 表示） ──
constexpr bool SENSOR_OIL_PRESSURE_PRESENT  = true;
constexpr bool SENSOR_WATER_TEMP_PRESENT    = true;
// 油温センサーを使用するかどうか
constexpr bool SENSOR_OIL_TEMP_PRESENT      = true;
constexpr bool SENSOR_AMBIENT_LIGHT_PRESENT = true;

// ── 色設定 (16 bit) ──
// RGB888 から 565 形式へ変換する constexpr 関数
constexpr uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b)
{
  return (static_cast<uint16_t>(r & 0xF8) << 8)
       | (static_cast<uint16_t>(g & 0xFC) << 3)
       | (static_cast<uint16_t>(b) >> 3);
}

constexpr uint16_t COLOR_WHITE  = rgb565(255, 255, 255);
constexpr uint16_t COLOR_BLACK  = rgb565(0,   0,   0);
constexpr uint16_t COLOR_ORANGE = rgb565(255, 165, 0);
constexpr uint16_t COLOR_YELLOW = rgb565(255, 255, 0);
constexpr uint16_t COLOR_RED    = rgb565(255,   0, 0);
constexpr uint16_t COLOR_GRAY   = rgb565(169, 169, 169);

// ── 油圧の表示設定 ──
// 数値表示の上限 (バー単位)
constexpr float MAX_OIL_PRESSURE_DISPLAY = 15.0f;
// メーター目盛の上限
constexpr float MAX_OIL_PRESSURE_METER   = 10.0f;

// ── 水温メーター設定 ──
// 水温メーター下限と上限を80℃〜105℃に設定
constexpr float WATER_TEMP_METER_MIN = 80.0f;
constexpr float WATER_TEMP_METER_MAX = 105.0f;

// ── 画面サイズ ──
constexpr int LCD_WIDTH  = 320;
constexpr int LCD_HEIGHT = 240;

// ── ALS/輝度自動制御 ──
enum class BrightnessMode { Day, Dusk, Night };

constexpr uint32_t LUX_THRESHOLD_DAY  = 15;
constexpr uint32_t LUX_THRESHOLD_DUSK = 10;

constexpr uint8_t  BACKLIGHT_DAY   = 255;
constexpr uint8_t  BACKLIGHT_DUSK  = 200;
constexpr uint8_t  BACKLIGHT_NIGHT =  60;

constexpr int MEDIAN_BUFFER_SIZE = 10;

// ── ADS1015 のチャンネル定義 ──
constexpr uint8_t ADC_CH_WATER_TEMP   = 0;
constexpr uint8_t ADC_CH_OIL_PRESSURE = 2;
constexpr uint8_t ADC_CH_OIL_TEMP     = 1;

// サンプリング数設定
constexpr int PRESSURE_SAMPLE_SIZE   = 5;
constexpr int WATER_TEMP_SAMPLE_SIZE = 2;  // 500ms間隔×10サンプルで約5秒平均
constexpr int OIL_TEMP_SAMPLE_SIZE   = 2;  // 500ms間隔×10サンプルで約5秒平均

#endif // CONFIG_H
