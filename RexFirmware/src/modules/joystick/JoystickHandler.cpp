#include "JoystickHandler.h"

JoystickHandler::JoystickHandler(MotionManager& motion) : _motion(motion) {}

void JoystickHandler::readAndProcess() {
    // 1. Process Mobility (Joystick 1)
    int rawX = analogRead(JOY1_X);
    int rawY = analogRead(JOY1_Y);
    
    int offsetX = rawX - JOYSTICK_CENTER;
    int offsetY = rawY - JOYSTICK_CENTER;
    
    float normX = 0.0f;
    float normY = 0.0f;
    
    if (abs(offsetX) >= JOYSTICK_DEADZONE) {
        float normalized = (float)abs(offsetX) / (float)JOYSTICK_CENTER;
        normalized = constrain(normalized, 0.0f, 1.0f);
        normX = normalized * normalized; // quadratic curve
        if (offsetX < 0) normX = -normX;
    }
    
    if (abs(offsetY) >= JOYSTICK_DEADZONE) {
        float normalized = (float)abs(offsetY) / (float)JOYSTICK_CENTER;
        normalized = constrain(normalized, 0.0f, 1.0f);
        normY = normalized * normalized; // quadratic curve
        if (offsetY < 0) normY = -normY;
    }
    
    // Drive the car: Y is throttle, X is steering
    _motion.driveCar(normX, normY);
    
    // 2. Process Camera Pan/Tilt (Joystick 2)
    processAxis(analogRead(JOY2_X), 0); // Index 0 is Camera Pan
    processAxis(analogRead(JOY2_Y), 1); // Index 1 is Camera Tilt
}

void JoystickHandler::processAxis(int rawValue, int servoIndex) {
    int offset = rawValue - JOYSTICK_CENTER;

    if (abs(offset) < JOYSTICK_DEADZONE) {
        return;
    }

    float normalized = (float)abs(offset) / (float)JOYSTICK_CENTER;
    normalized = constrain(normalized, 0.0f, 1.0f);

    // Apply quadratic curve for smoother control at low stick deflection
    float intensity = normalized * normalized;
    
    // Maintain direction
    if (offset < 0) {
        intensity = -intensity;
    }

    _motion.moveJoint(servoIndex, intensity);
}
