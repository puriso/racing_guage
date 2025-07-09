#ifndef MENU_H
#define MENU_H

#include <M5Unified.h>
#include <SD.h>
#include "config.h"

struct AppConfig {
    bool showOilPressure;
    bool showWaterTemp;
    bool showOilTemp;
    bool debugMode;
};

extern AppConfig appConfig;

void loadConfig();
void saveConfig();
void showMenu();

#endif // MENU_H
