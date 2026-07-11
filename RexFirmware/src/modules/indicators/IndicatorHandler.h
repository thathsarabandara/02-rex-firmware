#ifndef INDICATOR_HANDLER_H
#define INDICATOR_HANDLER_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "../../../include/Config.h"
#include "../../../include/RobotStates.h"
#include "LedControl.h"
#include "BuzzerControl.h"

class IndicatorHandler {
public:
    IndicatorHandler();
    void begin();
    void update();
    
    // Core state management
    void setState(OperatingState state);

private:
    Adafruit_NeoPixel _frontPixels;
    Adafruit_NeoPixel _backPixels;
    OperatingState _currentState;
    unsigned long _lastUpdate;
    int _animationFrame;
    
    LedControl _leds;
    BuzzerControl _buzzer;
};

#endif // INDICATOR_HANDLER_H
