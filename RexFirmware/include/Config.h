#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ======================================================
// ROBOT IDENTITY & VERSION
// ======================================================
#define ROBOT_ID                "REX-47-ROBOT-CAR"
#define SERIAL_KEY              "1234-5678-9012-3456"
#define FIRMWARE_VERSION        "1.1.0"

// ======================================================
// I2C BUS CONFIGURATION
// ======================================================
#define I2C_DISPLAY_SDA         21
#define I2C_DISPLAY_SCL         22
#define I2C_SENSOR_SDA          32
#define I2C_SENSOR_SCL          33
#define I2C_CLOCK_SPEED         100000 // 100kHz for stability

// ======================================================
// I2C DEVICE ADDRESSES
// ======================================================
#define PCA9685_ADDR            0x40  // Servo Driver Address
#define MCP23017_ADDR           0x20  // GPIO Expander Address
#define MPU6050_ADDR            0x68  // Gyroscope/Accelerometer Address
#define OLED_I2C_ADDR           0x3C  // OLED Display Address (usually 0x3C or 0x3D)
#define INA226_ADDR             0x45  // INA226 Power Monitor Address (A0/A1 tied to select 0x45)

// ======================================================
// OLED DISPLAY DRIVER SELECTION
// ======================================================
// Some displays sold as SH1106 actually use SSD1306, and vice versa.
// If your display shows shifted, jagged, or corrupted graphics, try switching the driver below.
#define DISPLAY_DRIVER_SH1106   0
#define DISPLAY_DRIVER_SSD1306  1
#define OLED_DISPLAY_DRIVER     DISPLAY_DRIVER_SH1106 // Default to SH1106. Change to DISPLAY_DRIVER_SSD1306 if display is jagy/shifted.

// ======================================================
// TB6612FNG MOTOR DRIVER PINS (MCP23017 & ESP32)
// ======================================================
// High-Speed PWMs remain on ESP32
#define MOTOR_LEFT_PWMA         14
#define MOTOR_RIGHT_PWMB        13

// Low-Speed Logic moved to MCP23017 Port B (Pins 8-15)
#define MOTOR_LEFT_AIN1         8   // GPB0
#define MOTOR_LEFT_AIN2         9   // GPB1
#define MOTOR_RIGHT_BIN1        10  // GPB2
#define MOTOR_RIGHT_BIN2        11  // GPB3
#define MOTOR_STBY              12  // GPB4

// ======================================================
// SERVO CHANNELS & LIMITS (PCA9685)
// ======================================================
#define SERVO_FREQ              50
#define SERVOMIN                125   // Out of 4096 (12-bit)
#define SERVOMAX                575   // Out of 4096 (12-bit)

#define PAN_SERVO_CH            0
#define PAN_MIN                 0
#define PAN_MAX                 180
#define PAN_HOME                90

#define TILT_SERVO_CH           1
#define TILT_MIN                70
#define TILT_MAX                140
#define TILT_HOME               90

// ======================================================
// IR LINE SENSORS (MCP23017 Pins GPA0-GPA3)
// ======================================================
#define IR_PIN_OUT1             0  // Left Outer (GPA0)
#define IR_PIN_OUT2             1  // Left Inner (GPA1)
#define IR_PIN_OUT3             2  // Right Inner (GPA2)
#define IR_PIN_OUT4             3  // Right Outer (GPA3)

// ======================================================
// ENVIRONMENT SENSOR (DHT22 / AHT22)
// ======================================================
#define DHT_PIN                 23
#define DHT_TYPE                DHT22

// ======================================================
// FRONT ULTRASONIC SENSOR (HC-SR04)
// ======================================================
#define ULTRASONIC_TRIG_PIN     18
#define ULTRASONIC_ECHO_PIN     19

// ======================================================
// INMP441 I2S MICROPHONE
// ======================================================
#define INMP441_SCK             25
#define INMP441_WS              26
#define INMP441_SD              27

// ======================================================
// MAX98357 I2S AMPLIFIER (SPEAKER)
// ======================================================
#define MAX98357_BCLK           16
#define MAX98357_LRC            15
#define MAX98357_DIN            2

// ======================================================
// WS2812 NEOPIXEL LED STRIPS
// ======================================================
#define NEOPIXEL_FRONT_PIN      17
#define NEOPIXEL_BACK_PIN       05
#define LED_COUNT               10

// ======================================================
// BUZZER PIN
// ======================================================
#define BUZZER_PIN              15

// ======================================================
// MOVEMENT SETTINGS
// ======================================================
const float DEFAULT_SMOOTH_FACTOR = 0.12f;
const float DEFAULT_MAX_SPEED     = 1.0f;
const int   UPDATE_DELAY_MS       = 20;

// ======================================================
// DATA STRUCTURES
// ======================================================
struct ServoData {
    uint8_t channel;
    float currentAngle;
    float targetAngle;
    int minAngle;
    int maxAngle;
};

// ======================================================
// NETWORK SETTINGS
// ======================================================
#define FALLBACK_WIFI_SSID      "TP-Link_D664"
#define FALLBACK_WIFI_PASSWORD  "Bandara@2001"

#define API_REGISTER_URL        "http://192.168.1.100:9000/api/v1/robots/register"

#define MQTT_BROKER             "192.168.1.100"
#define MQTT_PORT               1883
#define MQTT_USER               "thathsara"
#define MQTT_PASS               "BandaPutha"

#endif // CONFIG_H
