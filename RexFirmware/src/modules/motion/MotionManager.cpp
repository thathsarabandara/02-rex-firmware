#include "MotionManager.h"
#include "../safety/SafetySystem.h"

MotionManager::MotionManager(ServoController& controller, MotorController& motorController) 
    : _controller(controller), _motorController(motorController), _smoothFactor(DEFAULT_SMOOTH_FACTOR) 
{
    // Initialize servo data with home positions and limits
    _servos[0] = {BASE_SERVO, BASE_HOME, BASE_HOME, BASE_MIN, BASE_MAX};
    _servos[1] = {SHOULDER_SERVO, SHOULDER_HOME, SHOULDER_HOME, SHOULDER_MIN, SHOULDER_MAX};
    _servos[2] = {ELBOW_SERVO, ELBOW_HOME, ELBOW_HOME, ELBOW_MIN, ELBOW_MAX};
    _servos[3] = {GRIPPER_SERVO, GRIPPER_HOME, GRIPPER_HOME, GRIPPER_MIN, GRIPPER_MAX};
}

void MotionManager::begin() {
    _controller.begin();
    _motorController.begin();
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
    for (int i = 0; i < 4; i++) {
        float error = _servos[i].targetAngle - _servos[i].currentAngle;

        if (abs(error) > 0.05f) {
            _servos[i].currentAngle += error * _smoothFactor;
            _controller.setAngle(_servos[i].channel, _servos[i].currentAngle);
        }
    }
}

void MotionManager::setTarget(int servoIndex, float newTarget) {
    if (servoIndex < 0 || servoIndex >= 4) return;
    
    _servos[servoIndex].targetAngle = constrain(
        newTarget, 
        _servos[servoIndex].minAngle, 
        _servos[servoIndex].maxAngle
    );
}

void MotionManager::moveJoint(int servoIndex, float intensity) {
    if (servoIndex < 0 || servoIndex >= 4) return;
    
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
    return (servoIndex >= 0 && servoIndex < 4) ? _servos[servoIndex].targetAngle : 0;
}

float MotionManager::getCurrent(int servoIndex) const {
    return (servoIndex >= 0 && servoIndex < 4) ? _servos[servoIndex].currentAngle : 0;
}

void MotionManager::startupSequence() {
    // Initial sync
    for (int i = 0; i < 4; i++) {
        _controller.setAngle(_servos[i].channel, _servos[i].currentAngle);
    }
    // Motor self-test sequence
    _motorController.drive(0.0f, 0.5f); // Move forward 50% speed
    delay(200);
    _motorController.drive(0.0f, -0.5f); // Move backward 50% speed
    delay(200);
    _motorController.stop();             // Stop motors

    delay(600);
    Serial.println("Motion System Initialized");
}
