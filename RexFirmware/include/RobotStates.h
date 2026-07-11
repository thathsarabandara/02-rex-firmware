#ifndef ROBOT_STATES_H
#define ROBOT_STATES_H

#include <Arduino.h>

// ======================================================
// ENUMS FOR ROBOT STATE SYSTEMS
// ======================================================

// 1. Core Operating States
enum OperatingState {
    OP_BOOTING,
    OP_IDLE_READY,
    OP_MANUAL_CONTROL,
    OP_AUTONOMOUS_PATROL,
    OP_LINE_FOLLOWING,
    OP_FOLLOW_ME,
    OP_AI_THINKING,
    OP_LISTENING,
    OP_SPEAKING,
    OP_CAMERA_RECORDING,
    OP_CHARGING,
    OP_FULLY_CHARGED,
    OP_LOW_BATTERY,
    OP_CRITICAL_BATTERY,
    OP_OBSTACLE_WARNING,
    OP_OBSTACLE_CRITICAL,
    OP_WIFI_CONNECTING,
    OP_MQTT_DISCONNECTED,
    OP_SENSOR_ERROR,
    OP_MOTOR_ERROR,
    OP_UNKNOWN_PERSON,
    OP_INTRUDER_DETECTED,
    OP_GAS_ALERT,
    OP_EMERGENCY_STOP,
    OP_SHUTDOWN,
    OP_STATE_COUNT // Helper count
};

// 2. Emotional States (face animation & LED color style modification)
enum EmotionState {
    EM_HAPPY,
    EM_NEUTRAL,
    EM_CURIOUS,
    EM_SLEEPY,
    EM_SURPRISED,
    EM_ANGRY,
    EM_SAD,
    EM_SCARED,
    EM_LOVING,
    EM_FOCUSED
};

// 3. Sub-system battery states
enum BatteryState {
    BATT_FULL,
    BATT_NORMAL,
    BATT_MEDIUM,
    BATT_LOW,
    BATT_CRITICAL,
    BATT_CHARGING,
    BATT_FULLY_CHARGED
};

// 4. Sub-system network states
enum NetworkState {
    NET_OFFLINE,
    NET_CONNECTING,
    NET_CONNECTED,
    NET_INTERNET_LOST,
    NET_CLOUD_CONNECTED,
    NET_OTA_UPDATING
};

// 5. Sub-system AI agent states
enum AIState {
    AI_WAITING,
    AI_WAKE_WORD,
    AI_RECORDING,
    AI_SPEECH_TO_TEXT,
    AI_THINKING,
    AI_FUNCTION_CALLING,
    AI_TEXT_TO_SPEECH,
    AI_SPEAKING,
    AI_FINISHED
};

// 6. Sub-system navigation states
enum NavState {
    NAV_STOPPED,
    NAV_MANUAL,
    NAV_EXPLORING,
    NAV_FOLLOWING_PERSON,
    NAV_GOING_TO_LOC,
    NAV_AVOIDING_OBSTACLE,
    NAV_DOCKING,
    NAV_CHARGING
};

// 7. Sub-system security states
enum SecurityState {
    SEC_NORMAL,
    SEC_MOTION_DETECTED,
    SEC_UNKNOWN_PERSON,
    SEC_KNOWN_PERSON,
    SEC_INTRUDER_ALERT,
    SEC_EMERGENCY
};

// ======================================================
// CONFIGURATION STRUCTURE MAPPING
// ======================================================
struct StateBehaviorConfig {
    OperatingState state;
    const char* name;
    const char* oledText;
};

// Array of operating states mappings
const StateBehaviorConfig STATE_BEHAVIOR_CONFIGS[OP_STATE_COUNT] = {
    { OP_BOOTING, "Booting", "Loading..." },
    { OP_IDLE_READY, "Idle Ready", "Ready" },
    { OP_MANUAL_CONTROL, "Manual Control", "Manual" },
    { OP_AUTONOMOUS_PATROL, "Autonomous Patrol", "Patrol" },
    { OP_LINE_FOLLOWING, "Line Following", "Line mode" },
    { OP_FOLLOW_ME, "Follow Me", "Following" },
    { OP_AI_THINKING, "AI Thinking", "Thinking..." },
    { OP_LISTENING, "Listening", "Listening" },
    { OP_SPEAKING, "Speaking", "Talking" },
    { OP_CAMERA_RECORDING, "Camera Recording", "REC" },
    { OP_CHARGING, "Charging", "Charging %" },
    { OP_FULLY_CHARGED, "Fully Charged", "Full" },
    { OP_LOW_BATTERY, "Low Battery", "Low battery" },
    { OP_CRITICAL_BATTERY, "Critical Battery", "Critical battery" },
    { OP_OBSTACLE_WARNING, "Obstacle Warning", "Obstacle near" },
    { OP_OBSTACLE_CRITICAL, "Obstacle Critical", "Stop: obstacle" },
    { OP_WIFI_CONNECTING, "WiFi Connecting", "WiFi..." },
    { OP_MQTT_DISCONNECTED, "MQTT Disconnected", "Cloud disconnected" },
    { OP_SENSOR_ERROR, "Sensor Error", "Sensor error" },
    { OP_MOTOR_ERROR, "Motor Error", "Motor fault" },
    { OP_UNKNOWN_PERSON, "Unknown Person", "Unknown person" },
    { OP_INTRUDER_DETECTED, "Intruder Detected", "Intruder" },
    { OP_GAS_ALERT, "Gas Alert", "Gas detected" },
    { OP_EMERGENCY_STOP, "Emergency Stop", "STOP" },
    { OP_SHUTDOWN, "Shutdown", "Goodbye" }
};

#endif // ROBOT_STATES_H
