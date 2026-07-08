#include "RobotNetwork.h"

RobotNetwork* RobotNetwork::_instance = nullptr;

RobotNetwork::RobotNetwork(MotionManager& motion, SensorManager& sensors) 
    : _motion(motion), _sensors(sensors), _mqttClient(_wifiClient), 
      _lastReconnectAttempt(0), _lastTelemetryPublish(0) 
{
    _instance = this;
}

void RobotNetwork::begin() {
    setupWiFi();
    if (WiFi.status() == WL_CONNECTED) {
        registerRobot();
        _mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
        _mqttClient.setCallback(RobotNetwork::mqttCallbackWrapper);
        connectMQTT();
    }
}

void RobotNetwork::setupWiFi() {
    Serial.println("Initializing WiFi...");
    
    WiFiManager wifiManager;
    wifiManager.setTimeout(120);
    
    if (!wifiManager.autoConnect("REX-47-AP", "password123")) {
        Serial.println("WiFiManager failed to connect or hit timeout.");
        Serial.println("Attempting fallback WiFi...");
        
        WiFi.mode(WIFI_STA);
        WiFi.begin(FALLBACK_WIFI_SSID, FALLBACK_WIFI_PASSWORD);
        
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            Serial.print(".");
            attempts++;
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nFallback WiFi connected!");
        } else {
            Serial.println("\nFallback WiFi failed.");
        }
    } else {
        Serial.println("\nWiFi connected via WiFiManager!");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    }
}

void RobotNetwork::registerRobot() {
    Serial.println("Registering robot with API Gateway...");
    HTTPClient http;
    
    http.begin(API_REGISTER_URL);
    http.addHeader("Content-Type", "application/json");
    
    StaticJsonDocument<256> doc;
    doc["robot_id"] = ROBOT_ID;
    doc["serial_key"] = SERIAL_KEY;
    doc["name"] = "REX-47 Robot Car";
    doc["model"] = "ESP32-REX-CAR";
    doc["firmware_version"] = FIRMWARE_VERSION;
    
    String payload;
    serializeJson(doc, payload);
    
    int httpResponseCode = http.POST(payload);
    
    if (httpResponseCode > 0) {
        Serial.printf("HTTP Response code: %d\n", httpResponseCode);
        String response = http.getString();
        Serial.println(response);
    } else {
        Serial.printf("Error code: %d\n", httpResponseCode);
    }
    
    http.end();
}

void RobotNetwork::connectMQTT() {
    Serial.print("Connecting to MQTT...");
    
    String clientId = "RexCarClient-";
    clientId += String(random(0xffff), HEX);
    
    String statusTopic = String("robots/") + ROBOT_ID + "/status";
    
    // Last Will and Testament: If disconnects, publish "OFFLINE"
    if (_mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASS, statusTopic.c_str(), 0, true, "OFFLINE")) {
        Serial.println("connected!");
        
        // Publish that we are online
        publishStatus("ONLINE");
        
        // Subscribe to commands
        String commandTopic = String("robots/") + ROBOT_ID + "/commands/#";
        _mqttClient.subscribe(commandTopic.c_str());
        Serial.printf("Subscribed to %s\n", commandTopic.c_str());
    } else {
        Serial.print("failed, rc=");
        Serial.print(_mqttClient.state());
        Serial.println(" try again in 5 seconds");
    }
}

void RobotNetwork::publishStatus(const char* status) {
    if (_mqttClient.connected()) {
        String statusTopic = String("robots/") + ROBOT_ID + "/status";
        _mqttClient.publish(statusTopic.c_str(), status, true);
    }
}

void RobotNetwork::publishTelemetry() {
    if (!_mqttClient.connected()) return;

    const RobotTelemetry& t = _sensors.getTelemetry();
    
    StaticJsonDocument<512> doc;
    doc["robot_id"] = ROBOT_ID;
    doc["timestamp"] = millis();
    
    JsonObject sensors = doc.createNestedObject("sensors");
    
    // IR Sensors
    JsonObject ir = sensors.createNestedObject("ir");
    ir["left_outer"] = t.irLeftOuter;
    ir["left_inner"] = t.irLeftInner;
    ir["right_inner"] = t.irRightInner;
    ir["right_outer"] = t.irRightOuter;
    
    // Environment
    JsonObject env = sensors.createNestedObject("env");
    env["temperature"] = t.temperature;
    env["humidity"] = t.humidity;
    
    // IMU
    JsonObject imu = sensors.createNestedObject("imu");
    JsonArray accel = imu.createNestedArray("accel");
    accel.add(t.accelX);
    accel.add(t.accelY);
    accel.add(t.accelZ);
    JsonArray gyro = imu.createNestedArray("gyro");
    gyro.add(t.gyroX);
    gyro.add(t.gyroY);
    gyro.add(t.gyroZ);
    
    // Power
    JsonObject power = sensors.createNestedObject("power");
    power["voltage"] = t.busVoltage;
    power["current"] = t.currentMA;
    power["power"] = t.powerMW;

    // Servo positions
    JsonObject camera = doc.createNestedObject("camera");
    camera["pan"] = _motion.getCurrent(0);
    camera["tilt"] = _motion.getCurrent(1);
    
    String payload;
    serializeJson(doc, payload);
    
    String telemetryTopic = String("robots/") + ROBOT_ID + "/telemetry";
    _mqttClient.publish(telemetryTopic.c_str(), payload.c_str());
}

void RobotNetwork::update() {
    if (WiFi.status() != WL_CONNECTED) {
        return; // Don't try MQTT if no WiFi
    }
    
    if (!_mqttClient.connected()) {
        unsigned long now = millis();
        if (now - _lastReconnectAttempt > 5000) {
            _lastReconnectAttempt = now;
            connectMQTT();
        }
    } else {
        _mqttClient.loop();
        
        // Publish Telemetry every 2 seconds
        unsigned long now = millis();
        if (now - _lastTelemetryPublish > 2000) {
            _lastTelemetryPublish = now;
            publishTelemetry();
        }
    }
}

bool RobotNetwork::isConnected() {
    return _mqttClient.connected();
}

String RobotNetwork::getIPAddress() {
    if (WiFi.status() == WL_CONNECTED) {
        return WiFi.localIP().toString();
    }
    return "0.0.0.0";
}

void RobotNetwork::handleMessage(char* topic, byte* payload, unsigned int length) {
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, payload, length);
    
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }
    
    // Convert topic to String for easier matching
    String topicStr = String(topic);
    
    // 1. Process car mobility command
    if (topicStr.endsWith("/commands/move")) {
        if (doc.containsKey("direction")) {
            String direction = doc["direction"].as<String>();
            float speed = doc.containsKey("speed") ? doc["speed"].as<float>() : 100.0f;
            _motion.driveCar(direction, speed);
            Serial.printf("MQTT Car Move: %s @ %.1f%%\n", direction.c_str(), speed);
        }
        else if (doc.containsKey("command") && doc["command"].as<String>() == "stop") {
            _motion.stopCar();
            Serial.println("MQTT Car Stop");
        }
        else if (doc.containsKey("x") && doc.containsKey("y")) {
            float x = doc["x"].as<float>();
            float y = doc["y"].as<float>();
            _motion.driveCar(x, y);
            Serial.printf("MQTT Car Direct Drive: X=%.2f, Y=%.2f\n", x, y);
        }
    }
    // 2. Process camera pan-tilt command
    else if (topicStr.endsWith("/commands/camera")) {
        // Direct pan/tilt key/values
        if (doc.containsKey("pan")) {
            float pan = doc["pan"].as<float>();
            _motion.setTarget(0, pan);
            Serial.printf("MQTT Camera Pan: %.2f\n", pan);
        }
        if (doc.containsKey("tilt")) {
            float tilt = doc["tilt"].as<float>();
            _motion.setTarget(1, tilt);
            Serial.printf("MQTT Camera Tilt: %.2f\n", tilt);
        }
        
        // Key/values like "servo" & "angle" for backward compatibility
        if (doc.containsKey("servo") && doc.containsKey("angle")) {
            float angle = doc["angle"].as<float>();
            if (doc["servo"].is<int>()) {
                int servoCh = doc["servo"].as<int>();
                if (servoCh == 0 || servoCh == PAN_SERVO_CH) {
                    _motion.setTarget(0, angle);
                } else if (servoCh == 1 || servoCh == TILT_SERVO_CH) {
                    _motion.setTarget(1, angle);
                }
            } else {
                String servoStr = doc["servo"].as<String>();
                if (servoStr.equalsIgnoreCase("pan")) {
                    _motion.setTarget(0, angle);
                } else if (servoStr.equalsIgnoreCase("tilt")) {
                    _motion.setTarget(1, angle);
                }
            }
        }
    }
}

void RobotNetwork::mqttCallbackWrapper(char* topic, byte* payload, unsigned int length) {
    if (_instance) {
        _instance->handleMessage(topic, payload, length);
    }
}
