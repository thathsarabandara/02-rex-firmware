#include "IndicatorHandler.h"
#include <driver/i2s.h>

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
    
    _frontPixels.setBrightness(50);
    _backPixels.setBrightness(50);
    
    _frontPixels.show();
    _backPixels.show();

    // Initialize I2S for MAX98357 Speaker on I2S_NUM_1
    i2s_config_t speaker_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };
    
    i2s_pin_config_t speaker_pins = {
        .bck_io_num = MAX98357_BCLK,
        .ws_io_num = MAX98357_LRC,
        .data_out_num = MAX98357_DIN,
        .data_in_num = I2S_PIN_NO_CHANGE
    };
    
    if (i2s_driver_install(I2S_NUM_1, &speaker_config, 0, NULL) == ESP_OK) {
        if (i2s_set_pin(I2S_NUM_1, &speaker_pins) == ESP_OK) {
            Serial.println(" -> MAX98357 Speaker (I2S) OK.");
        } else {
            Serial.println(" -> MAX98357 Speaker (I2S) Pin Config FAILED.");
        }
    } else {
        Serial.println(" -> MAX98357 Speaker (I2S) Driver FAILED.");
    }
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
