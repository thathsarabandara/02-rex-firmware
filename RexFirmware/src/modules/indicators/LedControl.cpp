#include "LedControl.h"

LedControl::LedControl() {}

void LedControl::setAllColor(Adafruit_NeoPixel& strip, uint32_t color) {
    for (uint16_t i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, color);
    }
}

uint32_t LedControl::wheel(Adafruit_NeoPixel& strip, byte pos) {
    pos = 255 - pos;
    if (pos < 85) {
        return strip.Color(255 - pos * 3, 0, pos * 3);
    }
    if (pos < 170) {
        pos -= 85;
        return strip.Color(0, pos * 3, 255 - pos * 3);
    }
    pos -= 170;
    return strip.Color(pos * 3, 255 - pos * 3, 0);
}

void LedControl::update(Adafruit_NeoPixel& frontStrip, Adafruit_NeoPixel& backStrip, OperatingState state, int frame) {
    uint32_t off = frontStrip.Color(0, 0, 0);
    uint32_t cyan = frontStrip.Color(0, 255, 255);
    uint32_t blue = frontStrip.Color(0, 0, 255);
    uint32_t red = frontStrip.Color(255, 0, 0);
    uint32_t green = frontStrip.Color(0, 255, 0);
    uint32_t yellow = frontStrip.Color(255, 255, 0);
    uint32_t white = frontStrip.Color(255, 255, 255);
    uint32_t orange = frontStrip.Color(255, 100, 0);
    uint32_t purple = frontStrip.Color(160, 0, 240);

    // Reset strips
    setAllColor(frontStrip, off);
    setAllColor(backStrip, off);

    int numLeds = frontStrip.numPixels();

    switch (state) {
        case OP_BOOTING: {
            // Cyan loading spinner
            int idx = frame % numLeds;
            frontStrip.setPixelColor(idx, cyan);
            frontStrip.setPixelColor((idx + 1) % numLeds, frontStrip.Color(0, 150, 150));
            backStrip.setPixelColor(idx, cyan);
            backStrip.setPixelColor((idx + 1) % numLeds, backStrip.Color(0, 150, 150));
            break;
        }

        case OP_IDLE_READY: {
            // Soft blue breathing
            int val = (int)(60 + 60 * sin(frame * 0.1));
            uint32_t softBlue = frontStrip.Color(0, 0, val);
            setAllColor(frontStrip, softBlue);
            setAllColor(backStrip, softBlue);
            break;
        }

        case OP_MANUAL_CONTROL: {
            // Front white, rear red
            setAllColor(frontStrip, white);
            setAllColor(backStrip, red);
            break;
        }

        case OP_AUTONOMOUS_PATROL: {
            // Purple slow sweep
            int pos = frame % (numLeds * 2);
            int ledPos = (pos < numLeds) ? pos : ((numLeds * 2 - 1) - pos);
            frontStrip.setPixelColor(ledPos, purple);
            backStrip.setPixelColor(ledPos, purple);
            break;
        }

        case OP_LINE_FOLLOWING: {
            // Green center flow (flows from sides to center)
            int flow = (frame / 2) % (numLeds / 2 + 1);
            int leftIdx = (numLeds / 2 - 1) - flow;
            int rightIdx = (numLeds / 2) + flow;
            if (leftIdx >= 0 && rightIdx < numLeds) {
                frontStrip.setPixelColor(leftIdx, green);
                frontStrip.setPixelColor(rightIdx, green);
                backStrip.setPixelColor(leftIdx, green);
                backStrip.setPixelColor(rightIdx, green);
            }
            break;
        }

        case OP_FOLLOW_ME: {
            // Blue tracking pulse
            int val = (int)(128 + 127 * sin(frame * 0.2));
            uint32_t pulseBlue = frontStrip.Color(0, 0, val);
            setAllColor(frontStrip, pulseBlue);
            setAllColor(backStrip, pulseBlue);
            break;
        }

        case OP_AI_THINKING: {
            // Purple pulse
            int val = (int)(128 + 127 * sin(frame * 0.25));
            uint32_t pulsePurple = frontStrip.Color(val * 0.6, 0, val);
            setAllColor(frontStrip, pulsePurple);
            setAllColor(backStrip, pulsePurple);
            break;
        }

        case OP_LISTENING: {
            // Blue wave
            for (int i = 0; i < numLeds; i++) {
                int val = (int)(127 + 127 * sin(frame * 0.3 + i * 0.8));
                frontStrip.setPixelColor(i, frontStrip.Color(0, val / 2, val));
                backStrip.setPixelColor(i, backStrip.Color(0, val / 2, val));
            }
            break;
        }

        case OP_SPEAKING: {
            // Cyan VU meter
            int height = frame % (numLeds / 2 + 1);
            for (int i = 0; i < height; i++) {
                frontStrip.setPixelColor(numLeds / 2 - 1 - i, cyan);
                frontStrip.setPixelColor(numLeds / 2 + i, cyan);
                backStrip.setPixelColor(numLeds / 2 - 1 - i, cyan);
                backStrip.setPixelColor(numLeds / 2 + i, cyan);
            }
            break;
        }

        case OP_CAMERA_RECORDING: {
            // Small red blink (pixel 0 blinks red, rest off)
            if ((frame / 6) % 2 == 0) {
                frontStrip.setPixelColor(0, red);
                backStrip.setPixelColor(0, red);
            }
            break;
        }

        case OP_CHARGING: {
            // Green fill up animation
            int fill = (frame / 3) % (numLeds + 1);
            for (int i = 0; i < fill; i++) {
                frontStrip.setPixelColor(i, green);
                backStrip.setPixelColor(i, green);
            }
            break;
        }

        case OP_FULLY_CHARGED: {
            // Solid green
            setAllColor(frontStrip, green);
            setAllColor(backStrip, green);
            break;
        }

        case OP_LOW_BATTERY: {
            // Yellow battery bar (only first 3 pixels lit yellow)
            for (int i = 0; i < 3; i++) {
                frontStrip.setPixelColor(i, yellow);
                backStrip.setPixelColor(i, yellow);
            }
            break;
        }

        case OP_CRITICAL_BATTERY: {
            // Red flashing
            if ((frame / 5) % 2 == 0) {
                setAllColor(frontStrip, red);
                setAllColor(backStrip, red);
            }
            break;
        }

        case OP_OBSTACLE_WARNING: {
            // Yellow front glow (back off)
            int val = (int)(100 + 100 * sin(frame * 0.1));
            setAllColor(frontStrip, frontStrip.Color(val, val, 0));
            break;
        }

        case OP_OBSTACLE_CRITICAL: {
            // Red front flash fast
            if ((frame / 3) % 2 == 0) {
                setAllColor(frontStrip, red);
            }
            break;
        }

        case OP_WIFI_CONNECTING: {
            // Orange spinner
            int idx = frame % numLeds;
            frontStrip.setPixelColor(idx, orange);
            backStrip.setPixelColor((idx + numLeds / 2) % numLeds, orange);
            break;
        }

        case OP_MQTT_DISCONNECTED: {
            // Orange double blink
            if ((frame / 8) % 3 < 2) {
                setAllColor(frontStrip, orange);
                setAllColor(backStrip, orange);
            }
            break;
        }

        case OP_SENSOR_ERROR: {
            // Red center blink
            if ((frame / 5) % 2 == 0) {
                frontStrip.setPixelColor(numLeds / 2 - 1, red);
                frontStrip.setPixelColor(numLeds / 2, red);
                backStrip.setPixelColor(numLeds / 2 - 1, red);
                backStrip.setPixelColor(numLeds / 2, red);
            }
            break;
        }

        case OP_MOTOR_ERROR: {
            // Red side flash (outermost pixels blink)
            if ((frame / 4) % 2 == 0) {
                frontStrip.setPixelColor(0, red);
                frontStrip.setPixelColor(numLeds - 1, red);
                backStrip.setPixelColor(0, red);
                backStrip.setPixelColor(numLeds - 1, red);
            }
            break;
        }

        case OP_UNKNOWN_PERSON: {
            // Red/white flash alternations
            if ((frame / 6) % 2 == 0) {
                setAllColor(frontStrip, red);
                setAllColor(backStrip, white);
            } else {
                setAllColor(frontStrip, white);
                setAllColor(backStrip, red);
            }
            break;
        }

        case OP_INTRUDER_DETECTED: {
            // Fast red/white alarm strobe
            if ((frame / 2) % 2 == 0) {
                setAllColor(frontStrip, red);
                setAllColor(backStrip, white);
            } else {
                setAllColor(frontStrip, white);
                setAllColor(backStrip, red);
            }
            break;
        }

        case OP_GAS_ALERT: {
            // Red/orange alarm alert
            if ((frame / 3) % 2 == 0) {
                setAllColor(frontStrip, red);
                setAllColor(backStrip, orange);
            } else {
                setAllColor(frontStrip, orange);
                setAllColor(backStrip, red);
            }
            break;
        }

        case OP_EMERGENCY_STOP: {
            // Bright red fast flash strobe
            if (frame % 2 == 0) {
                setAllColor(frontStrip, red);
                setAllColor(backStrip, red);
            }
            break;
        }

        case OP_SHUTDOWN: {
            // Cyan fade out
            int val = constrain(255 - frame * 4, 0, 255);
            uint32_t fadeCyan = frontStrip.Color(0, val, val);
            setAllColor(frontStrip, fadeCyan);
            setAllColor(backStrip, fadeCyan);
            break;
        }
    }
}
