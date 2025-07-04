#include "settings.h"

bool debugModeEnabled = DEFAULT_DEBUG_MODE_ENABLED;
bool oilPressureEnabled = DEFAULT_SENSOR_OIL_PRESSURE_ENABLED;
bool waterTempEnabled = DEFAULT_SENSOR_WATER_TEMP_ENABLED;
bool oilTempEnabled = DEFAULT_SENSOR_OIL_TEMP_ENABLED;

static const char* SETTINGS_FILE = "/settings.txt";

// ────────────────────── 設定読み込み ──────────────────────
void loadSettings()
{
    if (!SD.begin(GPIO_NUM_4, SPI, 40000000)) {
        return; // SD初期化失敗
    }
    File f = SD.open(SETTINGS_FILE, FILE_READ);
    if (!f) return;

    while (f.available()) {
        String line = f.readStringUntil('\n');
        int eq = line.indexOf('=');
        if (eq < 0) continue;
        String key = line.substring(0, eq);
        String val = line.substring(eq + 1);
        bool flag = val.toInt() != 0;
        if (key == "debug") debugModeEnabled = flag;
        else if (key == "oilP") oilPressureEnabled = flag;
        else if (key == "waterT") waterTempEnabled = flag;
        else if (key == "oilT") oilTempEnabled = flag;
    }
    f.close();
}

// ────────────────────── 設定保存 ──────────────────────
void saveSettings()
{
    if (!SD.begin(GPIO_NUM_4, SPI, 40000000)) {
        return; // SD初期化失敗
    }
    SD.remove(SETTINGS_FILE);
    File f = SD.open(SETTINGS_FILE, FILE_WRITE);
    if (!f) return;

    f.printf("debug=%d\n", debugModeEnabled ? 1 : 0);
    f.printf("oilP=%d\n", oilPressureEnabled ? 1 : 0);
    f.printf("waterT=%d\n", waterTempEnabled ? 1 : 0);
    f.printf("oilT=%d\n", oilTempEnabled ? 1 : 0);
    f.close();
}
