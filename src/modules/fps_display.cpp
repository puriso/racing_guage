#include "fps_display.h"
#include "display.h"

// FPSラベルが描画済みかどうかを保持
static bool fpsLabelDrawn = false;

// ────────────────────── FPS表示 ──────────────────────
void drawFpsOverlay()
{
    mainCanvas.setFont(&fonts::Font0);
    mainCanvas.setTextSize(0);

    if (!fpsLabelDrawn) {
        // 表示領域を初期化してラベルを描画
        mainCanvas.fillRect(0, LCD_HEIGHT - 16, 80, 16, COLOR_BLACK);
        mainCanvas.setCursor(5, LCD_HEIGHT - 16);
        mainCanvas.println("FPS:");
        fpsLabelDrawn = true;
    }

    // 数値表示部のみ塗り直して更新
    mainCanvas.fillRect(5, LCD_HEIGHT - 8, 30, 8, COLOR_BLACK);
    mainCanvas.setCursor(5, LCD_HEIGHT - 8);
    mainCanvas.printf("%d", currentFramesPerSecond);
}
