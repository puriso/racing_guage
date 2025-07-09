#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>

struct AppSettings {
    bool showOilPressure;
    bool showWaterTemp;
    bool showOilTemp;
    bool debugMode;
};

extern AppSettings settings;

void loadSettings();
void saveSettings();

#endif // SETTINGS_H
