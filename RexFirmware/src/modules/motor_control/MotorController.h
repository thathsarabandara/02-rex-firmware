#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>
#include "../../../include/Config.h"

class MotorController {
public:
    MotorController();
    void begin();
    void drive(float x, float y);
    void drive(const String& direction, float speed);
    void stop();

private:
    void setMotorSpeeds(float leftSpeed, float rightSpeed);
};

#endif // MOTOR_CONTROLLER_H
