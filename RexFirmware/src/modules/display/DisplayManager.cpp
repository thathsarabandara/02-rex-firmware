#include "DisplayManager.h"

DisplayManager::DisplayManager() 
    : _display(U8G2_R0, /* reset=*/ U8X8_PIN_NONE), _initialized(false) {}

bool DisplayManager::begin() {
#if OLED_DISPLAY_DRIVER == DISPLAY_DRIVER_SH1106
    Serial.println("Initializing DisplayManager (U8G2 SH1106 I2C)...");
#elif OLED_DISPLAY_DRIVER == DISPLAY_DRIVER_SSD1306
    Serial.println("Initializing DisplayManager (U8G2 SSD1306 I2C)...");
#endif

    // Set fast I2C bus speed (400kHz) for smooth rendering
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
    _display.clearBuffer();
    
    _display.setFont(u8g2_font_6x12_tf);
    _display.drawStr(10, 15, "REX-47");
    _display.drawStr(10, 32, "ROBOT CAR BOOTING");
    _display.drawStr(10, 50, "Please Wait...");
    
    _display.sendBuffer();
}

void DisplayManager::update(OperatingState state, const RobotTelemetry& telemetry, const String& ipAddr, bool mqttConnected, int frame) {
    if (!_initialized) return;

    _display.clearBuffer();
    
    if (state == OP_BOOTING) {
        // Booting screen
        _display.setFont(u8g2_font_6x12_tf);
        _display.drawStr(10, 15, "REX-47");
        _display.drawStr(10, 32, "ROBOT CAR BOOTING");
        _display.drawStr(10, 50, "Please Wait...");
    } 
    else if (state == OP_OTA_UPDATE) {
        // OTA Update screen
        _display.setFont(u8g2_font_6x12_tf);
        _display.drawStr(10, 15, "OTA UPDATE");
        _display.drawFrame(10, 30, 108, 12);
        int barWidth = (frame % 60) * 104 / 60;
        _display.drawBox(12, 32, barWidth, 8);
    } 
    else {
        // Normal states show ONLY the animated eyes of the robot!
        OledEyeAnim anim = STATE_BEHAVIOR_CONFIGS[state].eyeAnim;
        drawProceduralEyes(anim, frame);
        
        // Overlay small indicators in special phases
        if (state == OP_BATTERY_LOW) {
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

// ======================================================
// PROCEDURAL EYE DRAWING FUNCTIONS
// ======================================================

void DisplayManager::drawLeftEye(int x, int y, int w, int h, int radius, bool filled) {
    if (filled) {
        _display.drawRBox(x - w/2, y - h/2, w, h, radius);
    } else {
        _display.drawRFrame(x - w/2, y - h/2, w, h, radius);
    }
}

void DisplayManager::drawRightEye(int x, int y, int w, int h, int radius, bool filled) {
    drawLeftEye(x, y, w, h, radius, filled);
}

void DisplayManager::drawHeart(int x, int y, int size) {
    // Draw heart shape centered at (x,y)
    _display.drawDisc(x - size/4, y - size/4, size/4);
    _display.drawDisc(x + size/4, y - size/4, size/4);
    _display.drawTriangle(x - size/2, y - size/8, x + size/2, y - size/8, x, y + size/2);
}

void DisplayManager::drawProceduralEyes(OledEyeAnim anim, int frame) {
    int lx = 42;  // Left eye center X
    int rx = 86;  // Right eye center X
    int ey = 38;  // Eye center Y
    
    int w = 22;   // Eye width
    int h = 20;   // Eye height
    int r = 6;    // Rounding radius

    switch (anim) {
        case EYE_LOADING: {
            int spinnerSize = 12;
            float angle = frame * 0.15f;
            int sx = 64 + spinnerSize * cos(angle);
            int sy = ey + spinnerSize * sin(angle);
            
            _display.drawCircle(64, ey, spinnerSize);
            _display.drawDisc(sx, sy, 4);
            
            drawLeftEye(lx, ey, 14, 12, 4);
            drawRightEye(rx, ey, 14, 12, 4);
            break;
        }
        case EYE_BLINK: {
            if (frame % 80 < 6) {
                // Closed eyes
                _display.drawHLine(lx - 10, ey, 20);
                _display.drawHLine(rx - 10, ey, 20);
            } else {
                drawLeftEye(lx, ey, w, h, r);
                drawRightEye(rx, ey, w, h, r);
                
                // Draw black pupil
                _display.setDrawColor(0);
                _display.drawDisc(lx, ey, 4);
                _display.drawDisc(rx, ey, 4);
                _display.setDrawColor(1);
            }
            break;
        }
        case EYE_WIDE: {
            drawLeftEye(lx, ey, w + 2, h + 4, r + 2);
            drawRightEye(rx, ey, w + 2, h + 4, r + 2);
            
            _display.setDrawColor(0);
            _display.drawDisc(lx, ey, 6);
            _display.drawDisc(rx, ey, 6);
            _display.setDrawColor(1);
            break;
        }
        case EYE_LOOK_UP: {
            drawLeftEye(lx, ey, w, h, r);
            drawRightEye(rx, ey, w, h, r);
            
            _display.setDrawColor(0);
            _display.drawDisc(lx, ey - 4, 4);
            _display.drawDisc(rx, ey - 4, 4);
            _display.setDrawColor(1);
            break;
        }
        case EYE_LOOK_LEFT_RIGHT: {
            drawLeftEye(lx, ey, w, h, r);
            drawRightEye(rx, ey, w, h, r);
            
            int dx = 5 * sin(frame * 0.08f);
            _display.setDrawColor(0);
            _display.drawDisc(lx + dx, ey, 4);
            _display.drawDisc(rx + dx, ey, 4);
            _display.setDrawColor(1);
            break;
        }
        case EYE_SHOCKED: {
            drawLeftEye(lx, ey, w + 4, h + 4, r + 2, false);
            drawRightEye(rx, ey, w + 4, h + 4, r + 2, false);
            
            _display.drawDisc(lx, ey, 2);
            _display.drawDisc(rx, ey, 2);
            break;
        }
        case EYE_SLEEPY: {
            _display.drawRBox(lx - w/2, ey, w, h/2, 3);
            _display.drawRBox(rx - w/2, ey, w, h/2, 3);
            break;
        }
        case EYE_TIRED: {
            _display.drawRFrame(lx - w/2, ey - h/4, w, h/2, 2);
            _display.drawRFrame(rx - w/2, ey - h/4, w, h/2, 2);
            
            _display.drawDisc(lx, ey, 2);
            _display.drawDisc(rx, ey, 2);
            break;
        }
        case EYE_X_EYES: {
            _display.drawLine(lx - 8, ey - 8, lx + 8, ey + 8);
            _display.drawLine(lx + 8, ey - 8, lx - 8, ey + 8);
            _display.drawLine(rx - 8, ey - 8, rx + 8, ey + 8);
            _display.drawLine(rx + 8, ey - 8, rx - 8, ey + 8);
            break;
        }
        case EYE_SEARCHING: {
            drawLeftEye(lx, ey, w, h, r, false);
            drawRightEye(rx, ey, w, h, r, false);
            
            float angle = frame * 0.12f;
            int dx = 5 * cos(angle);
            int dy = 5 * sin(angle);
            _display.drawDisc(lx + dx, ey + dy, 3);
            _display.drawDisc(rx + dx, ey + dy, 3);
            break;
        }
        case EYE_HAPPY: {
            int offset = 2 * sin(frame * 0.25f);
            
            for (int i = -1; i <= 1; i++) {
                _display.drawCircle(lx, ey + 8 + offset, 12);
                _display.drawCircle(rx, ey + 8 + offset, 12);
            }
            
            _display.setDrawColor(0);
            _display.drawBox(0, ey - 10, 128, 18 + offset);
            _display.setDrawColor(1);
            break;
        }
        case EYE_PROGRESS: {
            _display.drawFrame(34, ey - 6, 60, 12);
            int barWidth = (frame % 60) * 56 / 60;
            _display.drawBox(36, ey - 4, barWidth, 8);
            break;
        }
        case EYE_CLOSED: {
            _display.drawHLine(lx - 9, ey + 2, 18);
            _display.drawHLine(rx - 9, ey + 2, 18);
            break;
        }
        case EYE_SMILE: {
            _display.drawCircle(lx, ey + 5, 8);
            _display.drawCircle(rx, ey + 5, 8);
            
            _display.setDrawColor(0);
            _display.drawBox(0, ey - 10, 128, 15);
            _display.setDrawColor(1);
            break;
        }
        case EYE_ANGRY: {
            drawLeftEye(lx, ey, w, h, r);
            drawRightEye(rx, ey, w, h, r);
            
            _display.setDrawColor(0);
            _display.drawDisc(lx, ey, 4);
            _display.drawDisc(rx, ey, 4);
            
            // Eyebrows
            _display.drawLine(lx - 12, ey - 10, lx + 6, ey - 4);
            _display.drawLine(lx - 12, ey - 9, lx + 6, ey - 3);
            _display.drawLine(rx + 12, ey - 10, rx - 6, ey - 4);
            _display.drawLine(rx + 12, ey - 9, rx - 6, ey - 3);
            _display.setDrawColor(1);
            break;
        }
        case EYE_SCANNING: {
            drawLeftEye(lx, ey, w, h, r, false);
            drawRightEye(rx, ey, w, h, r, false);
            
            int sweepY = ey - 8 + (frame % 32) * 16 / 32;
            _display.drawHLine(lx - 9, sweepY, 18);
            _display.drawHLine(rx - 9, sweepY, 18);
            break;
        }
        case EYE_SAD: {
            drawLeftEye(lx, ey, w, h, r);
            drawRightEye(rx, ey, w, h, r);
            
            _display.setDrawColor(0);
            _display.drawDisc(lx, ey, 4);
            _display.drawDisc(rx, ey, 4);
            
            // Eyebrows
            _display.drawLine(lx - 10, ey - 4, lx + 8, ey - 10);
            _display.drawLine(lx - 10, ey - 3, lx + 8, ey - 9);
            _display.drawLine(rx + 10, ey - 4, rx - 8, ey - 10);
            _display.drawLine(rx + 10, ey - 3, rx - 8, ey - 9);
            _display.setDrawColor(1);
            break;
        }
        case EYE_SCARED: {
            int jx = (frame % 2 == 0) ? -1 : 1;
            int jy = (frame % 3 == 0) ? -1 : 1;
            
            drawLeftEye(lx + jx, ey + jy, w - 2, h - 2, r - 1);
            drawRightEye(rx + jx, ey + jy, w - 2, h - 2, r - 1);
            
            _display.setDrawColor(0);
            _display.drawDisc(lx + jx, ey + jy, 3);
            _display.drawDisc(rx + jx, ey + jy, 3);
            _display.setDrawColor(1);
            break;
        }
        case EYE_LOVING: {
            drawHeart(lx, ey, 14);
            drawHeart(rx, ey, 14);
            break;
        }
        case EYE_FOCUSED: {
            drawLeftEye(lx, ey, w, 10, 2);
            drawRightEye(rx, ey, w, 10, 2);
            
            _display.setDrawColor(0);
            _display.drawDisc(lx, ey, 3);
            _display.drawDisc(rx, ey, 3);
            _display.setDrawColor(1);
            break;
        }
    }
}
