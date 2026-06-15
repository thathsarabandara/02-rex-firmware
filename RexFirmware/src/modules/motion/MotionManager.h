#ifndef MOTION_MANAGER_H
#define MOTION_MANAGER_H

#include "../../../include/Config.h"
#include "../servo_control/ServoController.h"
#include "../motor_control/MotorController.h"

class MotionManager {
public:
    MotionManager(ServoController& controller, MotorController& motorController);

    void begin();
    void update();
    void setTarget(int servoIndex, float newTarget);
    void moveJoint(int servoIndex, float intensity);
    float getTarget(int servoIndex) const;
    float getCurrent(int servoIndex) const;
    
    // Car mobility controls
    void driveCar(float x, float y);
    void driveCar(const String& direction, float speed);
    void stopCar();
    
    void startupSequence();

private:
    ServoController& _controller;
    MotorController& _motorController;
    ServoData _servos[4];
    float _smoothFactor;
};

#endif // MOTION_MANAGER_H
