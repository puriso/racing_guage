#include "menu.h"
#include <M5Unified.h>

static bool menuVisible = false;

// ────────────────────── メニュー描画 ──────────────────────
static void drawMenu()
{
    const int X = 40, Y = 40, W = 240, H = 160;
    mainCanvas.fillRect(X, Y, W, H, COLOR_BLACK);
    mainCanvas.drawRect(X, Y, W, H, COLOR_WHITE);

    int line = Y + 20;
    auto drawItem = [&](const char* name, bool value) {
        mainCanvas.setCursor(X + 10, line);
        mainCanvas.printf("%s: %s", name, value ? "ON" : "OFF");
        line += 20;
    };

    drawItem("OIL.P", oilPressureEnabled);
    drawItem("OIL.T", oilTempEnabled);
    drawItem("WATER.T", waterTempEnabled);
    drawItem("DEBUG", debugModeEnabled);

    mainCanvas.setCursor(X + 10, line + 10);
    mainCanvas.print("SAVE");

    mainCanvas.pushSprite(0, 0);
}

// ────────────────────── タッチ処理 ──────────────────────
void handleMenu()
{
    M5.update();
    if (!menuVisible) {
        if (M5.Touch.getCount()) {
            menuVisible = true;
            drawMenu();
        }
        return;
    }

    if (!M5.Touch.getCount()) return;
    auto p = M5.Touch.getPressPoint();
    const int X = 40, Y = 40;

    int index = (p.y - (Y + 20)) / 20;
    if (p.x < X || p.x > X + 240) return;

    switch (index) {
    case 0: oilPressureEnabled = !oilPressureEnabled; drawMenu(); break;
    case 1: oilTempEnabled = !oilTempEnabled; drawMenu(); break;
    case 2: waterTempEnabled = !waterTempEnabled; drawMenu(); break;
    case 3: debugModeEnabled = !debugModeEnabled; drawMenu(); break;
    default:
        if (p.y > Y + 100) { // SAVE area
            saveSettings();
            menuVisible = false;
        }
        break;
    }
}
