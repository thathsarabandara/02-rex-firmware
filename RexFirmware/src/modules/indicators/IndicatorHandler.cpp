#include "IndicatorHandler.h"

IndicatorHandler::IndicatorHandler() 
    : _frontPixels(LED_COUNT, NEOPIXEL_FRONT_PIN, NEO_GRB + NEO_KHZ800),
      _backPixels(LED_COUNT, NEOPIXEL_BACK_PIN, NEO_GRB + NEO_KHZ800),
      _currentState(OP_BOOTING),
      _lastUpdate(0),
      _animationFrame(0) {}

void IndicatorHandler::begin() {
    _buzzer.begin();

    _frontPixels.begin();
    _backPixels.begin();
    
    _frontPixels.setBrightness(30);
    _backPixels.setBrightness(30);
    
    _frontPixels.show();
    _backPixels.show();

}


void IndicatorHandler::setState(OperatingState state) {
    if (_currentState != state) {
        _currentState = state;
        _animationFrame = 0;
        
        // Delegate state change to buzzer to trigger one-shot sound effects
        _buzzer.setState(state);
    }
}

void IndicatorHandler::update() {
    unsigned long now = millis();
    if (now - _lastUpdate < 40) return; // 25 FPS animations
    _lastUpdate = now;
    _animationFrame++;

    // Delegate LED update
    _leds.update(_frontPixels, _backPixels, _currentState, _animationFrame);
    
    _frontPixels.show();
    _backPixels.show();
    
    // Delegate buzzer update (for continuous sounds/alarms)
    _buzzer.update(_currentState, _animationFrame);
}
