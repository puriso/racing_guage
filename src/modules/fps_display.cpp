#include "fps_display.h"
#include "display.h"

// FPSラベルが描画済みかどうかを保持
static bool fpsLabelDrawn = false;

// ────────────────────── FPS表示 ──────────────────────
void drawFpsOverlay()
{
    mainCanvas.setFont(&fonts::Font0);
    mainCanvas.setTextSize(0);

    // ラベルは画面下部の表示と被らないよう少し上へ移動
    constexpr int FPS_Y = LCD_HEIGHT - 32;  // もとの表示より16px上

    if (!fpsLabelDrawn) {
        // 表示領域を初期化してラベルを描画
        mainCanvas.fillRect(0, FPS_Y, 80, 16, COLOR_BLACK);
        mainCanvas.setCursor(5, FPS_Y);
        mainCanvas.println("FPS:");
        fpsLabelDrawn = true;
    }

    // 数値表示部のみ塗り直して更新
    mainCanvas.fillRect(5, FPS_Y + 8, 30, 8, COLOR_BLACK);
    mainCanvas.setCursor(5, FPS_Y + 8);
    mainCanvas.printf("%d", currentFramesPerSecond);
}
