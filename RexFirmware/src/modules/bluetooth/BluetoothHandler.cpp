#include "BluetoothHandler.h"

BluetoothHandler::BluetoothHandler(MotionManager& motion) 
    : _motion(motion), _deviceConnected(false), _pServer(nullptr), _pCharacteristic(nullptr) {
}

void BluetoothHandler::begin(const char* deviceName) {
    // 1. Initialize BLE Environment
    BLEDevice::init(deviceName);

    // 2. Create BLE Server
    _pServer = BLEDevice::createServer();
    _pServer->setCallbacks(this);

    // 3. Create BLE Service
    BLEService *pService = _pServer->createService(SERVICE_UUID);

    // 4. Create BLE Characteristic
    _pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_WRITE_NR |
        BLECharacteristic::PROPERTY_NOTIFY |
        BLECharacteristic::PROPERTY_INDICATE
    );

    _pCharacteristic->setCallbacks(this);
    
    // Add descriptor for notifications
    _pCharacteristic->addDescriptor(new BLE2902());

    // 5. Start the service
    pService->start();

    // 6. Start advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    
    Serial.println("BLE System Started. Waiting for connections...");
}

bool BluetoothHandler::isConnected() const {
    return _deviceConnected;
}

void BluetoothHandler::onConnect(BLEServer* pServer) {
    _deviceConnected = true;
    Serial.println("BLE Client Connected.");
}

void BluetoothHandler::onDisconnect(BLEServer* pServer) {
    _deviceConnected = false;
    Serial.println("BLE Client Disconnected.");
    // Restart advertising when disconnected
    BLEDevice::startAdvertising();
}

void BluetoothHandler::onWrite(BLECharacteristic* pCharacteristic) {
    String value = pCharacteristic->getValue().c_str();
    value.trim();

    if (value.length() > 0) {
        Serial.print("BLE Received: ");
        Serial.println(value);

        if (value.startsWith("ESTOP") || value == "E" || value.startsWith("E:")) {
            _motion.stopCar();
            Serial.println("BLE Emergency Stop Executed!");
            return;
        }

        // Protocol expected: J:<servo_index>:<angle>\n
        // Examples:
        // J:0:90  -> Pan to 90
        // J:1:120 -> Tilt to 120
        
        int firstColon = value.indexOf(':');
        int secondColon = value.indexOf(':', firstColon + 1);

        if (value.startsWith("M:")) {
            // Format: M:<x>:<y> or M:<direction>:<speed>
            int firstColon = value.indexOf(':');
            int secondColon = value.indexOf(':', firstColon + 1);
            
            if (firstColon != -1 && secondColon != -1) {
                String param1 = value.substring(firstColon + 1, secondColon);
                String param2 = value.substring(secondColon + 1);
                
                param2.trim();
                
                bool isNumeric = true;
                if (param1.length() == 0) {
                    isNumeric = false;
                } else {
                    for (unsigned int i = 0; i < param1.length(); i++) {
                        char c = param1.charAt(i);
                        if (!isDigit(c) && c != '.' && c != '-') {
                            isNumeric = false;
                            break;
                        }
                    }
                }
                
                if (isNumeric) {
                    float x = param1.toFloat();
                    float y = param2.toFloat();
                    _motion.driveCar(x, y);
                    Serial.printf("BLE Car Direct Drive: X=%.2f, Y=%.2f\n", x, y);
                } else {
                    float speed = param2.toFloat();
                    _motion.driveCar(param1, speed);
                    Serial.printf("BLE Car Move: %s @ %.1f%%\n", param1.c_str(), speed);
                }
            }
        } else if (firstColon != -1 && secondColon != -1 && value.startsWith("J")) {
            String indexStr = value.substring(firstColon + 1, secondColon);
            String angleStr = value.substring(secondColon + 1);
            
            int servoIndex = indexStr.toInt();
            float targetAngle = angleStr.toFloat();

            if (servoIndex >= 0 && servoIndex < 2) {
                _motion.setTarget(servoIndex, targetAngle);
                Serial.printf("Setting Camera Servo %d to %.1f\n", servoIndex, targetAngle);
            } else {
                Serial.println("Error: Invalid camera servo index received via BLE.");
            }
        } else if (value.startsWith("A:")) {
            // Protocol expected: A:<pan_angle>:<tilt_angle>\n
            int c1 = value.indexOf(':', 2);

            if (c1 != -1) {
                float pan = value.substring(2, c1).toFloat();
                float tilt = value.substring(c1 + 1).toFloat();

                _motion.setTarget(0, pan);
                _motion.setTarget(1, tilt);
                Serial.printf("BLE Dual Camera Target: Pan=%.1f, Tilt=%.1f\n", pan, tilt);
            }
        }
    }
}
