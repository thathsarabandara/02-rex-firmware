#include "MotorController.h"

MotorController::MotorController() : _mcp(nullptr) {}

void MotorController::begin(Adafruit_MCP23X17* mcp) {
    _mcp = mcp;

    // Configure ESP32 PWM pins
    pinMode(MOTOR_LEFT_PWMA, OUTPUT);
    pinMode(MOTOR_RIGHT_PWMB, OUTPUT);

    // Configure MCP23017 direction and STBY pins
    if (_mcp != nullptr) {
        _mcp->pinMode(MOTOR_LEFT_AIN1, OUTPUT);
        _mcp->pinMode(MOTOR_LEFT_AIN2, OUTPUT);
        _mcp->pinMode(MOTOR_RIGHT_BIN1, OUTPUT);
        _mcp->pinMode(MOTOR_RIGHT_BIN2, OUTPUT);
        _mcp->pinMode(MOTOR_STBY, OUTPUT);

        // Disable standby to enable the driver board
        _mcp->digitalWrite(MOTOR_STBY, HIGH);
    }

    // Ensure motors are initially stopped
    stop();
}

void MotorController::drive(float x, float y) {
    // x is steering (-1.0 to 1.0), y is throttle (-1.0 to 1.0)
    // Mix them for differential steering
    float leftSpeed = y + x;
    float rightSpeed = y - x;

    // Constrain within motor limits
    leftSpeed = constrain(leftSpeed, -1.0f, 1.0f);
    rightSpeed = constrain(rightSpeed, -1.0f, 1.0f);

    setMotorSpeeds(leftSpeed, rightSpeed);
}

void MotorController::drive(const String& direction, float speed) {
    // Speed is expected to be 0 to 100. Normalize to 0.0 to 1.0.
    float normalizedSpeed = constrain(speed / 100.0f, 0.0f, 1.0f);

    if (direction.equalsIgnoreCase("forward")) {
        drive(0.0f, normalizedSpeed);
    } else if (direction.equalsIgnoreCase("backward")) {
        drive(0.0f, -normalizedSpeed);
    } else if (direction.equalsIgnoreCase("left")) {
        // Spin left in place
        drive(-normalizedSpeed, 0.0f);
    } else if (direction.equalsIgnoreCase("right")) {
        // Spin right in place
        drive(normalizedSpeed, 0.0f);
    } else {
        stop();
    }
}

void MotorController::stop() {
    setMotorSpeeds(0.0f, 0.0f);
}

void MotorController::setMotorSpeeds(float leftSpeed, float rightSpeed) {
    if (!_mcp) return; // Prevent crashes if MCP is not initialized

    // Left Motor Control (Motor A)
    if (leftSpeed > 0.01f) {
        _mcp->digitalWrite(MOTOR_LEFT_AIN1, HIGH);
        _mcp->digitalWrite(MOTOR_LEFT_AIN2, LOW);
        analogWrite(MOTOR_LEFT_PWMA, (int)(leftSpeed * 255));
    } else if (leftSpeed < -0.01f) {
        _mcp->digitalWrite(MOTOR_LEFT_AIN1, LOW);
        _mcp->digitalWrite(MOTOR_LEFT_AIN2, HIGH);
        analogWrite(MOTOR_LEFT_PWMA, (int)(-leftSpeed * 255));
    } else {
        _mcp->digitalWrite(MOTOR_LEFT_AIN1, LOW);
        _mcp->digitalWrite(MOTOR_LEFT_AIN2, LOW);
        analogWrite(MOTOR_LEFT_PWMA, 0);
    }

    // Right Motor Control (Motor B)
    if (rightSpeed > 0.01f) {
        _mcp->digitalWrite(MOTOR_RIGHT_BIN1, HIGH);
        _mcp->digitalWrite(MOTOR_RIGHT_BIN2, LOW);
        analogWrite(MOTOR_RIGHT_PWMB, (int)(rightSpeed * 255));
    } else if (rightSpeed < -0.01f) {
        _mcp->digitalWrite(MOTOR_RIGHT_BIN1, LOW);
        _mcp->digitalWrite(MOTOR_RIGHT_BIN2, HIGH);
        analogWrite(MOTOR_RIGHT_PWMB, (int)(-rightSpeed * 255));
    } else {
        _mcp->digitalWrite(MOTOR_RIGHT_BIN1, LOW);
        _mcp->digitalWrite(MOTOR_RIGHT_BIN2, LOW);
        analogWrite(MOTOR_RIGHT_PWMB, 0);
    }
}
