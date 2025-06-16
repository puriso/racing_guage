#ifndef CONFIG_H
#define CONFIG_H

// ────────────────────── 設定 ──────────────────────
// デバッグモードを有効にするかどうか
constexpr bool DEBUG_MODE_ENABLED = false;

// ── センサー接続可否（false にするとその項目は常に 0 表示） ──
constexpr bool SENSOR_OIL_PRESSURE_PRESENT  = true;
constexpr bool SENSOR_WATER_TEMP_PRESENT    = true;
constexpr bool SENSOR_OIL_TEMP_PRESENT      = false;
constexpr bool SENSOR_AMBIENT_LIGHT_PRESENT = true;

// ── 色設定 (16 bit) ──
constexpr uint16_t COLOR_WHITE  = M5.Lcd.color565(255, 255, 255);
constexpr uint16_t COLOR_BLACK  = M5.Lcd.color565(0,   0,   0);
constexpr uint16_t COLOR_ORANGE = M5.Lcd.color565(255, 165, 0);
constexpr uint16_t COLOR_YELLOW = M5.Lcd.color565(255, 255, 0);
constexpr uint16_t COLOR_RED    = M5.Lcd.color565(255,   0, 0);
constexpr uint16_t COLOR_GRAY   = M5.Lcd.color565(169, 169, 169);

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

#endif // CONFIG_H
