#include "menu.h"
#include "../modules/display.h"

// グローバル設定変数
AppConfig appConfig = {true, true, true, DEBUG_MODE_ENABLED};

// 設定ファイルのパス
static const char* CONFIG_PATH = "/config.txt";

// ────────────────────── 設定読み込み ──────────────────────
void loadConfig()
{
    if (!SD.begin(GPIO_NUM_4, SPI, 25000000)) {
        Serial.println("SD init failed");
        return;
    }
    File f = SD.open(CONFIG_PATH, FILE_READ);
    if (!f) return;
    while (f.available()) {
        String line = f.readStringUntil('\n');
        if (line.startsWith("oil_pressure="))
            appConfig.showOilPressure = line.substring(13).toInt();
        else if (line.startsWith("water_temp="))
            appConfig.showWaterTemp = line.substring(11).toInt();
        else if (line.startsWith("oil_temp="))
            appConfig.showOilTemp = line.substring(9).toInt();
        else if (line.startsWith("debug="))
            appConfig.debugMode = line.substring(6).toInt();
    }
    f.close();
}

// ────────────────────── 設定保存 ──────────────────────
void saveConfig()
{
    if (!SD.begin(GPIO_NUM_4, SPI, 25000000)) {
        Serial.println("SD init failed");
        return;
    }
    File f = SD.open(CONFIG_PATH, FILE_WRITE);
    if (!f) return;
    f.printf("oil_pressure=%d\n", appConfig.showOilPressure);
    f.printf("water_temp=%d\n", appConfig.showWaterTemp);
    f.printf("oil_temp=%d\n", appConfig.showOilTemp);
    f.printf("debug=%d\n", appConfig.debugMode);
    f.close();
}

// ────────────────────── メニュー描画 ──────────────────────
static void drawMenu()
{
    display.fillScreen(COLOR_BLACK);
    display.setTextFont(1);
    display.setTextSize(2);
    display.setCursor(10, 20);
    display.printf("油圧表示: %s", appConfig.showOilPressure ? "ON" : "OFF");
    display.setCursor(10, 50);
    display.printf("水温表示: %s", appConfig.showWaterTemp ? "ON" : "OFF");
    display.setCursor(10, 80);
    display.printf("油温表示: %s", appConfig.showOilTemp ? "ON" : "OFF");
    display.setCursor(10, 110);
    display.printf("デバッグ: %s", appConfig.debugMode ? "ON" : "OFF");
    display.setCursor(10, 160);
    display.print("SAVE をタップ");
}

// ────────────────────── メニュー表示 ──────────────────────
void showMenu()
{
    drawMenu();
    while (true) {
        M5.update();
        auto detail = M5.Touch.getDetail();
        if (detail.wasPressed()) {
            auto t = detail;
            if (t.y < 40) {
                appConfig.showOilPressure = !appConfig.showOilPressure;
                drawMenu();
            } else if (t.y < 70) {
                appConfig.showWaterTemp = !appConfig.showWaterTemp;
                drawMenu();
            } else if (t.y < 100) {
                appConfig.showOilTemp = !appConfig.showOilTemp;
                drawMenu();
            } else if (t.y < 130) {
                appConfig.debugMode = !appConfig.debugMode;
                drawMenu();
            } else {
                saveConfig();
                break;
            }
            while (M5.Touch.getDetail().isPressed()) M5.update();
        }
    }
    display.fillScreen(COLOR_BLACK);
}

