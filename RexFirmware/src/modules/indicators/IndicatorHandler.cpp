#include "IndicatorHandler.h"

IndicatorHandler::IndicatorHandler() 
    : _frontPixels(LED_COUNT, NEOPIXEL_FRONT_PIN, NEO_GRB + NEO_KHZ800),
      _backPixels(LED_COUNT, NEOPIXEL_BACK_PIN, NEO_GRB + NEO_KHZ800),
      _currentState(OP_BOOTING),
      _lastUpdate(0),
      _animationFrame(0) {}

void IndicatorHandler::begin() {
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    _frontPixels.begin();
    _backPixels.begin();
    
    _frontPixels.setBrightness(50);
    _backPixels.setBrightness(50);
    
    _frontPixels.show();
    _backPixels.show();
}

void IndicatorHandler::setState(OperatingState state) {
    if (_currentState != state) {
        _currentState = state;
        _animationFrame = 0;
        
        // Auto-play buzzer sound mapped to this state
        BuzzerSound sound = STATE_BEHAVIOR_CONFIGS[state].buzzer;
        if (sound != SOUND_NONE) {
            playSound(sound);
        }
    }
}

void IndicatorHandler::update() {
    unsigned long now = millis();
    if (now - _lastUpdate < 40) return; // 25 FPS animations
    _lastUpdate = now;
    _animationFrame++;

    // Fetch the target patterns for front and rear strips
    LedPattern frontPat = STATE_BEHAVIOR_CONFIGS[_currentState].frontLed;
    LedPattern rearPat = STATE_BEHAVIOR_CONFIGS[_currentState].rearLed;

    // Apply the patterns to both strips
    applyPattern(_frontPixels, frontPat, true);
    applyPattern(_backPixels, rearPat, false);
    
    _frontPixels.show();
    _backPixels.show();
    
    // Play non-blocking siren if in Alarm Mode
    if (_currentState == OP_ALARM_MODE) {
        // Fast sweep siren
        int freq = 800 + 400 * sin(_animationFrame * 0.3f);
        ledcAttach(BUZZER_PIN, freq, 10);
        ledcWriteTone(BUZZER_PIN, freq);
    } else if (STATE_BEHAVIOR_CONFIGS[_currentState].buzzer == SOUND_NONE) {
        ledcWriteTone(BUZZER_PIN, 0);
        ledcDetach(BUZZER_PIN);
    }
}

void IndicatorHandler::applyPattern(Adafruit_NeoPixel& strip, LedPattern pattern, bool isFront) {
    uint32_t off = strip.Color(0, 0, 0);
    
    switch (pattern) {
        case LED_OFF:
            setAllColor(strip, off);
            break;
            
        case LED_BREATHING_BLUE: {
            int val = (int)(127 + 127 * sin(_animationFrame * 0.1));
            setAllColor(strip, strip.Color(0, 0, val));
            break;
        }
        
        case LED_BREATHING_WHITE: {
            int val = (int)(60 + 60 * sin(_animationFrame * 0.08)); // dimmer white for soft breathing
            setAllColor(strip, strip.Color(val, val, val));
            break;
        }
        
        case LED_SPINNING_BLUE: {
            setAllColor(strip, off);
            int idx = _animationFrame % LED_COUNT;
            strip.setPixelColor(idx, strip.Color(0, 50, 255));
            strip.setPixelColor((idx + 1) % LED_COUNT, strip.Color(0, 0, 100));
            break;
        }
        
        case LED_ROTATING_CYAN: {
            setAllColor(strip, off);
            int idx = (_animationFrame / 2) % LED_COUNT;
            strip.setPixelColor(idx, strip.Color(0, 255, 255));
            strip.setPixelColor((idx + 1) % LED_COUNT, strip.Color(0, 150, 150));
            strip.setPixelColor((idx + 2) % LED_COUNT, strip.Color(0, 50, 50));
            break;
        }
        
        case LED_PULSE_GREEN: {
            int val = (int)(128 + 127 * sin(_animationFrame * 0.2));
            setAllColor(strip, strip.Color(0, val, 0));
            break;
        }
        
        case LED_FLASH_BLUE: {
            uint32_t blue = strip.Color(0, 0, 255);
            setAllColor(strip, (_animationFrame % 4 < 2) ? blue : off);
            break;
        }
        
        case LED_MOVING_WHITE: {
            // Chase lights
            setAllColor(strip, off);
            int idx = _animationFrame % LED_COUNT;
            strip.setPixelColor(idx, strip.Color(200, 200, 200));
            strip.setPixelColor((idx + 5) % LED_COUNT, strip.Color(200, 200, 200));
            break;
        }
        
        case LED_DIRECTION_WHITE: {
            // Sweep outwards from center
            setAllColor(strip, off);
            int sweep = _animationFrame % 5;
            strip.setPixelColor(4 - sweep, strip.Color(200, 200, 200));
            strip.setPixelColor(5 + sweep, strip.Color(200, 200, 200));
            break;
        }
        
        case LED_FLASH_RED: {
            uint32_t red = strip.Color(255, 0, 0);
            setAllColor(strip, (_animationFrame % 6 < 3) ? red : off);
            break;
        }
        
        case LED_CHASE_ORANGE: {
            setAllColor(strip, off);
            int idx = (_animationFrame) % LED_COUNT;
            uint32_t orange = strip.Color(255, 100, 0);
            strip.setPixelColor(idx, orange);
            strip.setPixelColor((idx + 1) % LED_COUNT, orange);
            break;
        }
        
        case LED_BREATHING_GREEN: {
            int val = (int)(127 + 127 * sin(_animationFrame * 0.08));
            setAllColor(strip, strip.Color(0, val, 0));
            break;
        }
        
        case LED_BREATHING_YELLOW: {
            int val = (int)(100 + 100 * sin(_animationFrame * 0.1));
            setAllColor(strip, strip.Color(val, val, 0));
            break;
        }
        
        case LED_BLINKING_RED: {
            uint32_t red = strip.Color(255, 0, 0);
            setAllColor(strip, (_animationFrame % 10 < 5) ? red : off);
            break;
        }
        
        case LED_CHASING_BLUE: {
            setAllColor(strip, off);
            int idx = (_animationFrame) % LED_COUNT;
            strip.setPixelColor(idx, strip.Color(0, 0, 255));
            strip.setPixelColor((idx + 2) % LED_COUNT, strip.Color(0, 0, 255));
            break;
        }
        
        case LED_FLASH_GREEN: {
            uint32_t green = strip.Color(0, 255, 0);
            setAllColor(strip, (_animationFrame % 8 < 4) ? green : off);
            break;
        }
        
        case LED_BREATHING_PURPLE: {
            int val = (int)(100 + 100 * sin(_animationFrame * 0.08));
            setAllColor(strip, strip.Color(val, 0, val));
            break;
        }
        
        case LED_RAINBOW_SWEEP: {
            for (int i = 0; i < LED_COUNT; i++) {
                strip.setPixelColor(i, wheel(((i * 256 / LED_COUNT) + _animationFrame * 4) & 255));
            }
            break;
        }
        
        case LED_FADE_BLUE: {
            int val = constrain(255 - _animationFrame * 5, 0, 255);
            setAllColor(strip, strip.Color(0, 0, val));
            break;
        }
        
        case LED_STROBE_RED: {
            uint32_t red = strip.Color(255, 0, 0);
            setAllColor(strip, (_animationFrame % 2 == 0) ? red : off);
            break;
        }
        
        case LED_SCANNER_WHITE: {
            setAllColor(strip, off);
            // Knight rider scanner back and forth
            int pos = _animationFrame % 18;
            int ledPos = (pos < 10) ? pos : (18 - pos);
            strip.setPixelColor(ledPos, strip.Color(255, 255, 255));
            if (ledPos > 0) strip.setPixelColor(ledPos - 1, strip.Color(100, 100, 100));
            if (ledPos < LED_COUNT - 1) strip.setPixelColor(ledPos + 1, strip.Color(100, 100, 100));
            break;
        }
        
        case LED_DIM_BLUE:
            setAllColor(strip, strip.Color(0, 0, 40));
            break;
    }
}

void IndicatorHandler::setAllColor(Adafruit_NeoPixel& strip, uint32_t color) {
    for (int i = 0; i < LED_COUNT; i++) {
        strip.setPixelColor(i, color);
    }
}

uint32_t IndicatorHandler::wheel(byte pos) {
    pos = 255 - pos;
    if (pos < 85) {
        return _frontPixels.Color(255 - pos * 3, 0, pos * 3);
    }
    if (pos < 170) {
        pos -= 85;
        return _frontPixels.Color(0, pos * 3, 255 - pos * 3);
    }
    pos -= 170;
    return _frontPixels.Color(pos * 3, 255 - pos * 3, 0);
}

void IndicatorHandler::playBeep(int freq, int durationMs) {
    if (freq > 0) {
        ledcAttach(BUZZER_PIN, freq, 10);
        ledcWriteTone(BUZZER_PIN, freq);
        delay(durationMs);
        ledcWriteTone(BUZZER_PIN, 0);
        ledcDetach(BUZZER_PIN);
    } else {
        delay(durationMs);
    }
}

void IndicatorHandler::playSound(BuzzerSound sound) {
    switch (sound) {
        case SOUND_STARTUP:
            playBeep(440, 100); delay(120);
            playBeep(554, 100); delay(120);
            playBeep(659, 100); delay(120);
            playBeep(880, 200);
            break;
            
        case SOUND_SHORT_BEEP:
            playBeep(1000, 80);
            break;
            
        case SOUND_CONFIRMATION:
            playBeep(880, 60); delay(80);
            playBeep(880, 80);
            break;
            
        case SOUND_WARNING:
            playBeep(300, 200);
            break;
            
        case SOUND_CHARGE_CONNECT:
            playBeep(523, 100); delay(120);
            playBeep(659, 100); delay(120);
            playBeep(784, 100); delay(120);
            playBeep(1047, 200);
            break;
            
        case SOUND_LOW_BATTERY:
            playBeep(392, 150); delay(200);
            playBeep(293, 150);
            break;
            
        case SOUND_ALARM:
            for (int i = 0; i < 3; i++) {
                playBeep(1500, 80); delay(100);
                playBeep(1000, 80); delay(100);
            }
            break;
            
        case SOUND_SUCCESS:
            playBeep(880, 100); delay(120);
            playBeep(1318, 200);
            break;
            
        case SOUND_GREETING:
            playBeep(587, 80); delay(100);
            playBeep(784, 80); delay(100);
            playBeep(988, 80); delay(100);
            playBeep(1175, 150);
            break;
            
        case SOUND_GOODBYE:
            playBeep(880, 100); delay(120);
            playBeep(659, 100); delay(120);
            playBeep(523, 100); delay(120);
            playBeep(349, 200);
            break;
            
        case SOUND_CONTINUOUS_ALARM:
            // Continuous siren is managed in update() loop non-blockingly
            break;
            
        default:
            break;
    }
}
