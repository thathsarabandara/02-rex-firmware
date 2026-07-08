#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include "../sensors/SensorManager.h"
#include "../../../include/Config.h"
#include "../../../include/RobotStates.h"

class DisplayManager {
public:
    DisplayManager();
    bool begin();
    void update(OperatingState state, const RobotTelemetry& telemetry, const String& ipAddr, bool mqttConnected, int frame);
    void clear();
    void showBooting();

private:
#if OLED_DISPLAY_DRIVER == DISPLAY_DRIVER_SH1106
    U8G2_SH1106_128X64_NONAME_F_HW_I2C _display;
#elif OLED_DISPLAY_DRIVER == DISPLAY_DRIVER_SSD1306
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C _display;
#else
    #error "Invalid OLED_DISPLAY_DRIVER specified in Config.h"
#endif
    bool _initialized;
    
    void drawHeader(const String& ipAddr, bool mqttConnected);
    void drawTelemetry(const RobotTelemetry& telemetry);
    void drawIRBar(const RobotTelemetry& telemetry);
    void drawProceduralEyes(OledEyeAnim anim, int frame);
    
    // Procedural eye helpers
    void drawLeftEye(int x, int y, int w, int h, int radius, bool filled = true);
    void drawRightEye(int x, int y, int w, int h, int radius, bool filled = true);
    void drawHeart(int x, int y, int size);
};

#endif // DISPLAY_MANAGER_H
