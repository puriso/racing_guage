#ifndef SETTINGS_H
#define SETTINGS_H

#include <SD.h>
#include "config.h"

// 設定をSDカードから読み込む
void loadSettings();

// 設定をSDカードへ保存する
void saveSettings();

#endif // SETTINGS_H
