#include "settings.h"
#include "config.h"
#include <M5CoreS3.h>
#include <SD.h>

// ────────────────────── デフォルト設定 ──────────────────────
AppSettings settings = {SENSOR_OIL_PRESSURE_PRESENT,
                        SENSOR_WATER_TEMP_PRESENT,
                        SENSOR_OIL_TEMP_PRESENT,
                        DEBUG_MODE_ENABLED};

static const char* SETTINGS_FILE = "/settings.txt";

// ────────────────────── 読み込み ──────────────────────
void loadSettings()
{
    if (!SD.begin()) return;
    File f = SD.open(SETTINGS_FILE, FILE_READ);
    if (!f) return;
    String line = f.readStringUntil('\n');
    if (line.length() > 0) settings.showOilPressure = line.toInt();
    line = f.readStringUntil('\n');
    if (line.length() > 0) settings.showWaterTemp = line.toInt();
    line = f.readStringUntil('\n');
    if (line.length() > 0) settings.showOilTemp = line.toInt();
    line = f.readStringUntil('\n');
    if (line.length() > 0) settings.debugMode = line.toInt();
    f.close();
}

// ────────────────────── 保存 ──────────────────────
void saveSettings()
{
    if (!SD.begin()) return;
    File f = SD.open(SETTINGS_FILE, FILE_WRITE | O_TRUNC);
    if (!f) return;
    f.printf("%d\n%d\n%d\n%d\n",
             settings.showOilPressure,
             settings.showWaterTemp,
             settings.showOilTemp,
             settings.debugMode);
    f.close();
}
