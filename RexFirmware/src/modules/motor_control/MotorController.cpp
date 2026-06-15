#include "MotorController.h"

MotorController::MotorController() {}

void MotorController::begin() {
    // Configure motor direction and PWM pins as outputs
    pinMode(MOTOR_LEFT_ENA, OUTPUT);
    pinMode(MOTOR_LEFT_IN1, OUTPUT);
    pinMode(MOTOR_LEFT_IN2, OUTPUT);
    pinMode(MOTOR_RIGHT_ENB, OUTPUT);
    pinMode(MOTOR_RIGHT_IN3, OUTPUT);
    pinMode(MOTOR_RIGHT_IN4, OUTPUT);

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
        // Spin left in place or steer left
        drive(-normalizedSpeed, normalizedSpeed * 0.5f);
    } else if (direction.equalsIgnoreCase("right")) {
        // Spin right in place or steer right
        drive(normalizedSpeed, normalizedSpeed * 0.5f);
    } else {
        stop();
    }
}

void MotorController::stop() {
    setMotorSpeeds(0.0f, 0.0f);
}

void MotorController::setMotorSpeeds(float leftSpeed, float rightSpeed) {
    // Left Motor Control
    if (leftSpeed > 0.01f) {
        digitalWrite(MOTOR_LEFT_IN1, HIGH);
        digitalWrite(MOTOR_LEFT_IN2, LOW);
        analogWrite(MOTOR_LEFT_ENA, (int)(leftSpeed * 255));
    } else if (leftSpeed < -0.01f) {
        digitalWrite(MOTOR_LEFT_IN1, LOW);
        digitalWrite(MOTOR_LEFT_IN2, HIGH);
        analogWrite(MOTOR_LEFT_ENA, (int)(-leftSpeed * 255));
    } else {
        digitalWrite(MOTOR_LEFT_IN1, LOW);
        digitalWrite(MOTOR_LEFT_IN2, LOW);
        analogWrite(MOTOR_LEFT_ENA, 0);
    }

    // Right Motor Control
    if (rightSpeed > 0.01f) {
        digitalWrite(MOTOR_RIGHT_IN3, HIGH);
        digitalWrite(MOTOR_RIGHT_IN4, LOW);
        analogWrite(MOTOR_RIGHT_ENB, (int)(rightSpeed * 255));
    } else if (rightSpeed < -0.01f) {
        digitalWrite(MOTOR_RIGHT_IN3, LOW);
        digitalWrite(MOTOR_RIGHT_IN4, HIGH);
        analogWrite(MOTOR_RIGHT_ENB, (int)(-rightSpeed * 255));
    } else {
        digitalWrite(MOTOR_RIGHT_IN3, LOW);
        digitalWrite(MOTOR_RIGHT_IN4, LOW);
        analogWrite(MOTOR_RIGHT_ENB, 0);
    }
}
