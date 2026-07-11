#include "BuzzerControl.h"

BuzzerControl::BuzzerControl() 
    : _lastState(OP_STATE_COUNT), _stateEntryTime(0), _hasBeepedOnce(false) {}

void BuzzerControl::begin() {
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
}

void BuzzerControl::playBeep(int freq, int durationMs) {
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

void BuzzerControl::setState(OperatingState state) {
    if (_lastState == state) return;
    
    _lastState = state;
    _stateEntryTime = millis();
    _hasBeepedOnce = false;

    // Trigger one-time beeps upon entering the state
    switch (state) {
        case OP_BOOTING:
            // 1 short beep
            playBeep(880, 100);
            break;

        case OP_FOLLOW_ME:
            // 1 beep only when target locked (here played once entering follow mode)
            playBeep(1000, 150);
            break;

        case OP_LISTENING:
            // 1 soft beep
            playBeep(600, 80);
            break;

        case OP_CAMERA_RECORDING:
            // 1 beep start/stop
            playBeep(900, 100);
            break;

        case OP_CHARGING:
            // 1 beep when connected
            playBeep(784, 150);
            break;

        case OP_FULLY_CHARGED:
            // 2 short beeps
            playBeep(880, 80);
            delay(80);
            playBeep(880, 80);
            break;

        case OP_LOW_BATTERY:
            // 2 slow beeps once
            playBeep(440, 200);
            delay(200);
            playBeep(440, 200);
            break;

        case OP_MQTT_DISCONNECTED:
            // 1 long beep once
            playBeep(350, 600);
            break;

        case OP_SENSOR_ERROR:
            // 3 short beeps
            for (int i = 0; i < 3; i++) {
                playBeep(1200, 80);
                if (i < 2) delay(80);
            }
            break;

        case OP_MOTOR_ERROR:
            // 3 fast beeps
            for (int i = 0; i < 3; i++) {
                playBeep(1500, 60);
                if (i < 2) delay(60);
            }
            break;

        case OP_UNKNOWN_PERSON:
            // Warning beep
            playBeep(440, 150);
            break;

        case OP_OBSTACLE_WARNING:
            // Optional short beep once
            playBeep(1000, 100);
            break;

        case OP_SHUTDOWN:
            // 1 long beep
            playBeep(220, 800);
            break;

        default:
            break;
    }
}

void BuzzerControl::update(OperatingState state, int frame) {
    // Handle continuous/repeating buzzer behaviors non-blockingly
    switch (state) {
        case OP_CRITICAL_BATTERY: {
            // Repeating warning beep (e.g. every 1.5 seconds)
            unsigned long elapsed = millis() - _stateEntryTime;
            if (elapsed % 1500 < 150) {
                ledcAttach(BUZZER_PIN, 500, 10);
                ledcWriteTone(BUZZER_PIN, 500);
            } else {
                ledcWriteTone(BUZZER_PIN, 0);
                ledcDetach(BUZZER_PIN);
            }
            break;
        }

        case OP_OBSTACLE_CRITICAL: {
            // Fast warning beeps
            unsigned long elapsed = millis() - _stateEntryTime;
            if (elapsed % 300 < 100) {
                ledcAttach(BUZZER_PIN, 1200, 10);
                ledcWriteTone(BUZZER_PIN, 1200);
            } else {
                ledcWriteTone(BUZZER_PIN, 0);
                ledcDetach(BUZZER_PIN);
            }
            break;
        }

        case OP_INTRUDER_DETECTED:
        case OP_GAS_ALERT: {
            // Alarm pattern (dual-tone siren sweep)
            int freq = 800 + 400 * sin(frame * 0.3f);
            ledcAttach(BUZZER_PIN, freq, 10);
            ledcWriteTone(BUZZER_PIN, freq);
            break;
        }

        case OP_EMERGENCY_STOP: {
            // Continuous fast high-pitched alert pattern
            unsigned long elapsed = millis() - _stateEntryTime;
            if (elapsed % 200 < 100) {
                ledcAttach(BUZZER_PIN, 2000, 10);
                ledcWriteTone(BUZZER_PIN, 2000);
            } else {
                ledcAttach(BUZZER_PIN, 1500, 10);
                ledcWriteTone(BUZZER_PIN, 1500);
            }
            break;
        }

        default:
            // Ensure buzzer is off for non-continuous states
            // Only turn off if not in a one-time startup beep phase (first 1000ms of entering state)
            if (millis() - _stateEntryTime > 1000) {
                ledcWriteTone(BUZZER_PIN, 0);
                ledcDetach(BUZZER_PIN);
            }
            break;
    }
}
