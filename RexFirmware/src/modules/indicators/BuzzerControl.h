#ifndef BUZZER_CONTROL_H
#define BUZZER_CONTROL_H

#include <Arduino.h>
#include "../../../include/Config.h"
#include "../../../include/RobotStates.h"

class BuzzerControl {
public:
    BuzzerControl();
    void begin();
    void setState(OperatingState state);
    void update(OperatingState state, int frame);
    void playBeep(int freq, int durationMs);

private:
    OperatingState _lastState;
    unsigned long _stateEntryTime;
    bool _hasBeepedOnce;
};

#endif // BUZZER_CONTROL_H
