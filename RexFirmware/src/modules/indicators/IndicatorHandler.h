#ifndef INDICATOR_HANDLER_H
#define INDICATOR_HANDLER_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "../../../include/Config.h"
#include "../../../include/RobotStates.h"

class IndicatorHandler {
public:
    IndicatorHandler();
    void begin();
    void update();
    
    // Core state management
    void setState(OperatingState state);
    void playSound(BuzzerSound sound);
    void playBeep(int freq, int durationMs);

private:
    Adafruit_NeoPixel _frontPixels;
    Adafruit_NeoPixel _backPixels;
    OperatingState _currentState;
    unsigned long _lastUpdate;
    int _animationFrame;
    
    // Animation execution helpers
    void applyPattern(Adafruit_NeoPixel& strip, LedPattern pattern, bool isFront);
    void setAllColor(Adafruit_NeoPixel& strip, uint32_t color);
    uint32_t wheel(byte pos);
};

#endif // INDICATOR_HANDLER_H
