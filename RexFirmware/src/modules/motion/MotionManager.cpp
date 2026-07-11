#include "MotionManager.h"
#include "../safety/SafetySystem.h"

MotionManager::MotionManager(ServoController& controller, MotorController& motorController) 
    : _controller(controller), _motorController(motorController), _smoothFactor(DEFAULT_SMOOTH_FACTOR) 
{
    // Initialize Camera Pan-Tilt servo configurations
    _servos[0] = {PAN_SERVO_CH, PAN_HOME, PAN_HOME, PAN_MIN, PAN_MAX};
    _servos[1] = {TILT_SERVO_CH, TILT_HOME, TILT_HOME, TILT_MIN, TILT_MAX};
}

void MotionManager::begin(Adafruit_MCP23X17* mcp) {
    _controller.begin();
    _motorController.begin(mcp);
    startupSequence();
}

void MotionManager::driveCar(float x, float y) {
    _motorController.drive(x, y);
}

void MotionManager::driveCar(const String& direction, float speed) {
    _motorController.drive(direction, speed);
}

void MotionManager::stopCar() {
    _motorController.stop();
}

void MotionManager::update() {
    // Smooth and update camera servos (0: Pan, 1: Tilt)
    for (int i = 0; i < 2; i++) {
        float error = _servos[i].targetAngle - _servos[i].currentAngle;

        if (abs(error) > 0.05f) {
            _servos[i].currentAngle += error * _smoothFactor;
            _controller.setAngle(_servos[i].channel, _servos[i].currentAngle);
        }
    }
}

void MotionManager::setTarget(int servoIndex, float newTarget) {
    if (servoIndex < 0 || servoIndex >= 2) return;
    
    _servos[servoIndex].targetAngle = constrain(
        newTarget, 
        _servos[servoIndex].minAngle, 
        _servos[servoIndex].maxAngle
    );
}

void MotionManager::moveJoint(int servoIndex, float intensity) {
    if (servoIndex < 0 || servoIndex >= 2) return;
    
    float direction = (intensity > 0) ? 1.0f : -1.0f;
    float absIntensity = abs(intensity);
    
    float safeSpeed = SafetySystem::calculateSafeSpeed(
        _servos[servoIndex].targetAngle,
        _servos[servoIndex].minAngle,
        _servos[servoIndex].maxAngle,
        direction,
        DEFAULT_MAX_SPEED
    );
    
    float movement = absIntensity * safeSpeed * direction;
    setTarget(servoIndex, _servos[servoIndex].targetAngle + movement);
}

float MotionManager::getTarget(int servoIndex) const {
    return (servoIndex >= 0 && servoIndex < 2) ? _servos[servoIndex].targetAngle : 0;
}

float MotionManager::getCurrent(int servoIndex) const {
    return (servoIndex >= 0 && servoIndex < 2) ? _servos[servoIndex].currentAngle : 0;
}

void MotionManager::startupSequence() {
    // Initial sync to home positions
    for (int i = 0; i < 2; i++) {
        _controller.setAngle(_servos[i].channel, _servos[i].currentAngle);
    }
    delay(300); // Give it a moment to reach home
    
    Serial.println("Performing Pan/Tilt self-test...");
    
    // Noticeable Pan sweep (Left to Right)
    _controller.setAngle(PAN_SERVO_CH, PAN_HOME - 45);
    delay(400);
    _controller.setAngle(PAN_SERVO_CH, PAN_HOME + 45);
    delay(400);
    _controller.setAngle(PAN_SERVO_CH, PAN_HOME);
    delay(400);

    // Noticeable Tilt sweep (Down to Up)
    _controller.setAngle(TILT_SERVO_CH, TILT_HOME - 30);
    delay(400);
    _controller.setAngle(TILT_SERVO_CH, TILT_HOME + 30);
    delay(400);
    _controller.setAngle(TILT_SERVO_CH, TILT_HOME);
    delay(400);

    // Noticeable Motor self-test sequence
    Serial.println("Performing Motor self-test...");
    _motorController.drive(0.0f, 0.6f);  // Move forward 60% speed
    delay(300);
    _motorController.drive(0.0f, -0.6f); // Move backward 60% speed
    delay(300);
    _motorController.drive(-0.6f, 0.0f); // Spin left 60% speed
    delay(250);
    _motorController.drive(0.6f, 0.0f);  // Spin right 60% speed
    delay(250);
    _motorController.stop();             // Stop motors

    Serial.println("Motion System Initialized");
}
