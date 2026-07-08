#include <Arduino.h>
#include <Wire.h>
#include "include/Config.h"
#include "include/RobotStates.h"
#include "src/modules/servo_control/ServoController.h"
#include "src/modules/motor_control/MotorController.h"
#include "src/modules/motion/MotionManager.h"
#include "src/modules/joystick/JoystickHandler.h"
#include "src/modules/bluetooth/BluetoothHandler.h"
#include "src/modules/network/RobotNetwork.h"
#include "src/modules/sensors/SensorManager.h"
#include "src/modules/display/DisplayManager.h"
#include "src/modules/indicators/IndicatorHandler.h"

// Set to true to run the state demo loop, cycling all 21 states
#define STATE_DEMO_LOOP_MODE    true

// ======================================================
// GLOBAL INSTANCES
// ======================================================
ServoController  servoCtrl(PCA9685_ADDR);
MotorController  motorCtrl;
MotionManager    motion(servoCtrl, motorCtrl);
JoystickHandler  joysticks(motion);
BluetoothHandler ble(motion);
SensorManager    sensors;
DisplayManager   displayMgr;
IndicatorHandler indicators;
RobotNetwork     network(motion, sensors);

// Operating State & Frame Timing
OperatingState currentState = OP_BOOTING;
unsigned long lastDemoTransition = 0;
unsigned long lastDisplayUpdate = 0;
int frameCounter = 0;

// ======================================================
// SETUP
// ======================================================
void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("REX-47 Restructured Firmware starting up...");

    // 1. Initialize Buzzer & LEDs
    indicators.begin();
    
    // 2. Initialize I2C Bus
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(I2C_CLOCK_SPEED);
    
    // 3. Initialize Display
    displayMgr.begin();
    displayMgr.showBooting();
    delay(500);

    // 4. Initialize Sensors
    sensors.begin();
    
    // 5. Initialize Motion System (Servos & Motors self-test)
    motion.begin();
    
    // 6. Initialize BLE Server
    ble.begin();
    
    // 7. Initialize WiFi/Network (if not in offline test demo)
    if (!STATE_DEMO_LOOP_MODE) {
        indicators.setState(OP_WIFI_CONNECTING);
        network.begin();
        if (network.isConnected()) {
            indicators.setState(OP_CONNECTED);
        } else {
            indicators.setState(OP_IDLE);
        }
    } else {
        Serial.println("DEMO LOOP MODE ENABLED. Cycling through all 21 states.");
        lastDemoTransition = millis();
        indicators.setState(currentState);
    }
}

// ======================================================
// MAIN LOOP
// ======================================================
void loop() {
    frameCounter++;

    if (STATE_DEMO_LOOP_MODE) {
        // Cycle states every 4 seconds
        if (millis() - lastDemoTransition > 4000) {
            lastDemoTransition = millis();
            int nextStateInt = (int)currentState + 1;
            if (nextStateInt >= OP_STATE_COUNT) {
                nextStateInt = 0; // Wrap around
            }
            currentState = (OperatingState)nextStateInt;
            indicators.setState(currentState);
            
            Serial.printf("Demo Transition -> State: %s (%d)\n", 
                STATE_BEHAVIOR_CONFIGS[currentState].name, 
                currentState
            );
        }

        // Update actuators and indicators
        indicators.update();

        // Throttle OLED updates to ~12 FPS (every 80 ms) to keep I2C bus free for sensors/servos
        if (millis() - lastDisplayUpdate >= 80) {
            lastDisplayUpdate = millis();
            displayMgr.update(currentState, sensors.getTelemetry(), network.getIPAddress(), network.isConnected(), frameCounter);
        }

        sensors.update(); // Keep sensors polling
        motion.update();  // Keep motion smoothing alive
    } else {
        // Standard operational code
        network.update();
        sensors.update();

        if (!ble.isConnected()) {
            joysticks.readAndProcess();
        }
        
        motion.update();
        
        if (network.isConnected()) {
            currentState = OP_CONNECTED;
        } else if (WiFi.status() == WL_CONNECTED) {
            currentState = OP_IDLE;
        } else {
            currentState = OP_WIFI_CONNECTING;
        }
        
        indicators.setState(currentState);
        indicators.update();

        // Throttle OLED updates to ~12 FPS (every 80 ms) to keep I2C bus free for sensors/servos
        if (millis() - lastDisplayUpdate >= 80) {
            lastDisplayUpdate = millis();
            displayMgr.update(currentState, sensors.getTelemetry(), network.getIPAddress(), network.isConnected(), frameCounter);
        }
    }

    delay(UPDATE_DELAY_MS);
}