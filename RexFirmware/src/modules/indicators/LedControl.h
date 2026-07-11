#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "../../../include/Config.h"
#include "../../../include/RobotStates.h"

class LedControl {
public:
    LedControl();
    void update(Adafruit_NeoPixel& frontStrip, Adafruit_NeoPixel& backStrip, OperatingState state, int frame);

private:
    void setAllColor(Adafruit_NeoPixel& strip, uint32_t color);
    uint32_t wheel(Adafruit_NeoPixel& strip, byte pos);
};

#endif // LED_CONTROL_H
