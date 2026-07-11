#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>
#include "../../../include/Config.h"

#include <Adafruit_MCP23X17.h>

class MotorController {
public:
    MotorController();
    void begin(Adafruit_MCP23X17* mcp);
    void drive(float x, float y);
    void drive(const String& direction, float speed);
    void stop();

private:
    Adafruit_MCP23X17* _mcp;
    void setMotorSpeeds(float leftSpeed, float rightSpeed);
};

#endif // MOTOR_CONTROLLER_H
