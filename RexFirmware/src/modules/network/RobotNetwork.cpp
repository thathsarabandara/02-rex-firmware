#include "RobotNetwork.h"

RobotNetwork* RobotNetwork::_instance = nullptr;

RobotNetwork::RobotNetwork(MotionManager& motion) 
    : _motion(motion), _mqttClient(_wifiClient), _lastReconnectAttempt(0) {
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
    
    // Use WiFiManager for dynamic provisioning
    WiFiManager wifiManager;
    
    // Set a timeout so it doesn't block forever if there's no input
    wifiManager.setTimeout(120);
    
    // Try to connect, if it fails after 120s timeout, it falls back to below logic
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
    
    StaticJsonDocument<200> doc;
    doc["robot_id"] = ROBOT_ID;
    doc["serial_key"] = SERIAL_KEY;
    doc["name"] = "REX-47 Robot";
    doc["model"] = "ESP32-REX";
    doc["firmware_version"] = "1.0.0";
    
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
    
    String clientId = "RexClient-";
    clientId += String(random(0xffff), HEX);
    
    String statusTopic = String("robots/") + ROBOT_ID + "/status";
    
    // Last Will and Testament: If disconnects, publish "OFFLINE"
    if (_mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASS, statusTopic.c_str(), 0, true, "OFFLINE")) {
        Serial.println("connected!");
        
        // Publish that we are online
        publishStatus("ONLINE");
        
        // Subscribe to commands
        String commandTopic = String("robots/") + ROBOT_ID + "/commands/move";
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
    }
}

bool RobotNetwork::isConnected() {
    return _mqttClient.connected();
}

void RobotNetwork::handleMessage(char* topic, byte* payload, unsigned int length) {
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, payload, length);
    
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }
    
    // 1. Check if it's a car mobility command
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
    // 2. Fallback to servo control for backward compatibility
    else if (doc.containsKey("servo") && doc.containsKey("angle")) {
        int servoIndex = doc["servo"];
        float angle = doc["angle"];
        
        // Apply bounds and update target
        if (servoIndex >= 0 && servoIndex <= 3) {
            _motion.setTarget(servoIndex, angle);
            Serial.printf("MQTT Servo Command: Servo %d -> %.2f\n", servoIndex, angle);
        }
    }
}

void RobotNetwork::mqttCallbackWrapper(char* topic, byte* payload, unsigned int length) {
    if (_instance) {
        _instance->handleMessage(topic, payload, length);
    }
}
