#include "DisplayManager.h"

DisplayManager::DisplayManager() 
    : _display(U8G2_R0, /* reset=*/ U8X8_PIN_NONE), _initialized(false) {}

bool DisplayManager::begin() {
#if OLED_DISPLAY_DRIVER == DISPLAY_DRIVER_SH1106
    Serial.println("Initializing DisplayManager (U8G2 SH1106 I2C)...");
#elif OLED_DISPLAY_DRIVER == DISPLAY_DRIVER_SSD1306
    Serial.println("Initializing DisplayManager (U8G2 SSD1306 I2C)...");
#endif

    // Set standard I2C bus speed (100kHz) for better stability when sharing the bus
    _display.setBusClock(400000);

    // Configure standard OLED address (usually 0x3C, which is 0x78 in 8-bit U8g2 format)
    _display.setI2CAddress(OLED_I2C_ADDR * 2);
    
    if (_display.begin()) {
        _display.clearBuffer();
        _display.setFont(u8g2_font_6x10_tf);
        _display.drawStr(10, 20, "Display Init OK");
        _display.sendBuffer();
        _initialized = true;
#if OLED_DISPLAY_DRIVER == DISPLAY_DRIVER_SH1106
        Serial.println(" -> U8g2 SH1106 OLED initialized successfully.");
#elif OLED_DISPLAY_DRIVER == DISPLAY_DRIVER_SSD1306
        Serial.println(" -> U8g2 SSD1306 OLED initialized successfully.");
#endif
        return true;
    }
    Serial.println(" -> ERROR: OLED Init Failed!");
    return false;
}

void DisplayManager::clear() {
    if (!_initialized) return;
    _display.clearBuffer();
    _display.sendBuffer();
}

void DisplayManager::showBooting() {
    if (!_initialized) return;
    
    // 1. Show REX text prominently
    _display.clearBuffer();
    _display.setFont(u8g2_font_ncenB14_tr);
    int textWidth = _display.getStrWidth("REX");
    _display.drawStr((128 - textWidth) / 2, 36, "REX"); // Centered vertically
    _display.sendBuffer();
    
    delay(1000); // Hold text for 1 second

    // 2. "Fade out" (Pixel wipe effect)
    _display.setDrawColor(0); // Draw black
    for (int i = 0; i < 64; i += 2) {
        _display.drawHLine(0, i, 128);
        _display.drawHLine(0, i + 1, 128);
        _display.sendBuffer();
        delay(15);
    }
    _display.setDrawColor(1); // Restore white
    _display.clearBuffer();
    
    delay(200);

    // 3. Animate the eye opening
    for (int frame = 0; frame < 20; frame++) {
        _display.clearBuffer();
        _eyes.draw(_display, OP_BOOTING, frame);
        _display.sendBuffer();
        delay(40); // 25 fps
    }
}

void DisplayManager::update(OperatingState state, const RobotTelemetry& telemetry, const String& ipAddr, bool mqttConnected, int frame) {
    if (!_initialized) return;

    _display.clearBuffer();
    
    // Draw the procedural eyes in the upper portion
    _eyes.draw(_display, state, frame);
    
    // Draw the state OLED text centered at the bottom of the screen (y=60)
    // Don't draw text during booting, just show the eye animation
    if (state == OP_BOOTING) {
        // Just the eyes! No text.
    } else if (state < OP_STATE_COUNT) {
        _display.setFont(u8g2_font_6x12_tf);
        const char* msg = STATE_BEHAVIOR_CONFIGS[state].oledText;
        
        // Handle charging percentage dynamically if in OP_CHARGING
        char buf[32];
        if (state == OP_CHARGING) {
            snprintf(buf, sizeof(buf), "Charging %d%%", (int)telemetry.busVoltage); // Or use custom telemetry percentage
            msg = buf;
        }
        
        int textWidth = _display.getStrWidth(msg);
        _display.drawStr((128 - textWidth) / 2, 60, msg);
    }
    
    // Overlay small indicators in special phases
    if (state == OP_LOW_BATTERY || state == OP_CRITICAL_BATTERY) {
        // Draw a small battery icon in the top right corner
        _display.drawFrame(110, 2, 15, 8);
        _display.drawBox(125, 4, 2, 4);
        if ((frame / 10) % 2 == 0) {
            _display.drawBox(112, 4, 3, 4);
        }
    } 
    else if (state == OP_CHARGING) {
        // Draw a small charging battery icon in the top right corner
        _display.drawFrame(110, 2, 15, 8);
        _display.drawBox(125, 4, 2, 4);
        int chargeWidth = (frame / 5) % 4;
        for (int i = 0; i < chargeWidth; i++) {
            _display.drawBox(112 + i * 3, 4, 2, 4);
        }
    }
    else if (state == OP_WIFI_CONNECTING) {
        // Draw a small antenna tower in top left corner (x=2, y=2)
        _display.drawLine(5, 10, 5, 4);
        _display.drawDisc(5, 2, 2);
        if ((frame / 10) % 2 == 0) {
            _display.drawLine(2, 4, 3, 5);
            _display.drawLine(8, 4, 7, 5);
        }
    }
    
    _display.sendBuffer();
}

void DisplayManager::drawHeader(const String& ipAddr, bool mqttConnected) {
    _display.setFont(u8g2_font_6x10_tf);
    _display.drawStr(0, 9, "REX-47");

    if (mqttConnected) {
        _display.drawStr(75, 9, "[MQTT:OK]");
    } else {
        _display.drawStr(75, 9, "[MQTT:NO]");
    }

    _display.drawHLine(0, 11, 128);
}

void DisplayManager::drawTelemetry(const RobotTelemetry& telemetry) {
    _display.setFont(u8g2_font_5x7_tf);
    
    char buf[32];
    snprintf(buf, sizeof(buf), "BAT: %.2fV | %d mA", telemetry.busVoltage, (int)telemetry.currentMA);
    _display.drawStr(0, 21, buf);
    
    snprintf(buf, sizeof(buf), "ENV: %.1f C | %.1f%%", telemetry.temperature, telemetry.humidity);
    _display.drawStr(0, 32, buf);
    
    snprintf(buf, sizeof(buf), "IMU: X:%.1f Y:%.1f", telemetry.accelX, telemetry.accelY);
    _display.drawStr(0, 43, buf);
}

void DisplayManager::drawIRBar(const RobotTelemetry& telemetry) {
    _display.setFont(u8g2_font_5x7_tf);
    _display.drawStr(0, 60, "IR: ");

    bool irStates[4] = {
        telemetry.irLeftOuter,
        telemetry.irLeftInner,
        telemetry.irRightInner,
        telemetry.irRightOuter
    };

    int startX = 35;
    int boxWidth = 16;
    int boxHeight = 10;
    int spacing = 5;

    for (int i = 0; i < 4; i++) {
        int x = startX + i * (boxWidth + spacing);
        int y = 52;
        _display.drawFrame(x, y, boxWidth, boxHeight);
        if (irStates[i]) {
            _display.drawBox(x + 2, y + 2, boxWidth - 4, boxHeight - 4);
        }
    }
}
