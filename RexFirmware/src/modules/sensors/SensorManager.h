#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MCP23X17.h>
#include <DHT.h>
#include <Adafruit_MPU6050.h>
#include <INA226_WE.h>
#include "../../../include/Config.h"

struct RobotTelemetry {
    // IR Line Sensors
    bool irLeftOuter;
    bool irLeftInner;
    bool irRightInner;
    bool irRightOuter;

    // Ultrasonic Sensor
    float distanceCM;

    // Environment
    float temperature;
    float humidity;

    // IMU MPU6050
    float accelX, accelY, accelZ;
    float gyroX, gyroY, gyroZ;
    float imuTemp;

    // Power INA226
    float busVoltage;
    float shuntVoltage;
    float currentMA;
    float powerMW;
};

class SensorManager {
public:
    SensorManager();
    bool begin();
    void update();
    const RobotTelemetry& getTelemetry() const;
    Adafruit_MCP23X17& getMCP(); // Expose MCP for motor control

private:
    Adafruit_MCP23X17 _mcp;
    DHT _dht;
    Adafruit_MPU6050 _mpu;
    INA226_WE _ina226;

    RobotTelemetry _telemetry;
    
    bool _mcpInitialized;
    bool _dhtInitialized;
    bool _mpuInitialized;
    bool _inaInitialized;

    unsigned long _lastUltrasonicRead;

    void readIRSensors();
    void readEnvironment();
    void readIMU();
    void readPower();
    void readUltrasonic();
};

#endif // SENSOR_MANAGER_H
