#include "SensorManager.h"
#include <driver/i2s.h>

Adafruit_MCP23X17& SensorManager::getMCP() {
    return _mcp;
}

SensorManager::SensorManager() 
    : _dht(DHT_PIN, DHT_TYPE), _ina226(&Wire1, INA226_ADDR), 
      _mcpInitialized(false), _dhtInitialized(false), 
      _mpuInitialized(false), _inaInitialized(false) 
{
    // Initialize clean telemetry
    memset(&_telemetry, 0, sizeof(RobotTelemetry));
    _telemetry.distanceCM = 999.0f; // Default safe distance
}

bool SensorManager::begin() {
    Serial.println("Initializing SensorManager...");

    // 0. Initialize Ultrasonic Pins
    pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
    pinMode(ULTRASONIC_ECHO_PIN, INPUT);
    _lastUltrasonicRead = 0;

    // 1. Initialize MCP23017 for IR Sensors
    if (_mcp.begin_I2C(MCP23017_ADDR, &Wire1)) {
        _mcp.pinMode(IR_PIN_OUT1, INPUT);
        _mcp.pinMode(IR_PIN_OUT2, INPUT);
        _mcp.pinMode(IR_PIN_OUT3, INPUT);
        _mcp.pinMode(IR_PIN_OUT4, INPUT);
        
        // Enable internal pull-ups if needed, but since IR sensors have active high/low outputs, 
        // standard input pin mode is correct.
        _mcpInitialized = true;
        Serial.println(" -> MCP23017 (IR) OK.");
    } else {
        Serial.println(" -> MCP23017 (IR) FAILED.");
    }

    // 2. Initialize DHT22
    _dht.begin();
    _dhtInitialized = true;
    Serial.println(" -> DHT22 OK.");

    // 3. Initialize MPU6050
    if (_mpu.begin(MPU6050_ADDR, &Wire1)) {
        _mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
        _mpu.setGyroRange(MPU6050_RANGE_500_DEG);
        _mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
        _mpuInitialized = true;
        Serial.println(" -> MPU6050 OK.");
    } else {
        Serial.println(" -> MPU6050 FAILED.");
    }

    // 4. Initialize INA226
    // INA226_WE needs .init() after instantiation
    if (_ina226.init()) {
        // Configure INA226 (shunt resistor is typically 0.1 Ohm or 0.002 Ohm on custom boards)
        // Let's configure it with standard settings: Average 16 samples, conversion time 1.1ms
        _ina226.setAverage(INA226_AVERAGE_16);
        _ina226.setConversionTime(INA226_CONV_TIME_1100);
        _ina226.setMeasureMode(INA226_CONTINUOUS);
        _inaInitialized = true;
        Serial.println(" -> INA226 OK.");
    } else {
        Serial.println(" -> INA226 FAILED.");
    }

    // 5. Initialize I2S for INMP441 Microphone
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = 16000,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };
    
    i2s_pin_config_t pin_config = {
        .bck_io_num = INMP441_SCK,
        .ws_io_num = INMP441_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = INMP441_SD
    };
    
    if (i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL) == ESP_OK) {
        if (i2s_set_pin(I2S_NUM_0, &pin_config) == ESP_OK) {
            Serial.println(" -> INMP441 (I2S) OK.");
        } else {
            Serial.println(" -> INMP441 (I2S) Pin Config FAILED.");
        }
    } else {
        Serial.println(" -> INMP441 (I2S) Driver FAILED.");
    }

    return (_mcpInitialized && _mpuInitialized && _inaInitialized);
}

void SensorManager::update() {
    readUltrasonic();
    readIRSensors();
    readEnvironment();
    readIMU();
    readPower();
}

const RobotTelemetry& SensorManager::getTelemetry() const {
    return _telemetry;
}

void SensorManager::readIRSensors() {
    if (!_mcpInitialized) return;
    
    // Read the pins (assuming active HIGH or active LOW)
    // Most IR sensors output LOW when reflecting off white (detect line) and HIGH off black (no line)
    // or vice versa. We store direct state.
    _telemetry.irLeftOuter = _mcp.digitalRead(IR_PIN_OUT1);
    _telemetry.irLeftInner = _mcp.digitalRead(IR_PIN_OUT2);
    _telemetry.irRightInner = _mcp.digitalRead(IR_PIN_OUT3);
    _telemetry.irRightOuter = _mcp.digitalRead(IR_PIN_OUT4);
}

void SensorManager::readEnvironment() {
    if (!_dhtInitialized) return;

    float t = _dht.readTemperature();
    float h = _dht.readHumidity();

    if (!isnan(t)) {
        _telemetry.temperature = t;
    }
    if (!isnan(h)) {
        _telemetry.humidity = h;
    }
}

void SensorManager::readIMU() {
    if (!_mpuInitialized) return;

    sensors_event_t a, g, temp;
    _mpu.getEvent(&a, &g, &temp);

    _telemetry.accelX = a.acceleration.x;
    _telemetry.accelY = a.acceleration.y;
    _telemetry.accelZ = a.acceleration.z;

    _telemetry.gyroX = g.gyro.x;
    _telemetry.gyroY = g.gyro.y;
    _telemetry.gyroZ = g.gyro.z;

    _telemetry.imuTemp = temp.temperature;
}

void SensorManager::readPower() {
    if (!_inaInitialized) return;

    // Read voltages and currents
    _telemetry.busVoltage = _ina226.getBusVoltage_V();
    _telemetry.shuntVoltage = _ina226.getShuntVoltage_mV();
    _telemetry.currentMA = _ina226.getCurrent_mA();
    _telemetry.powerMW = _ina226.getBusPower();
}

void SensorManager::readUltrasonic() {
    // Only read every 50ms to prevent ultrasonic echoes from overlapping
    if (millis() - _lastUltrasonicRead < 50) return;
    _lastUltrasonicRead = millis();

    // Send a 10us HIGH pulse to trigger
    digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(ULTRASONIC_TRIG_PIN, LOW);

    // Read the echo pulse length. 
    // Timeout set to 12000us (~2 meters) to prevent blocking the main loop too long.
    unsigned long duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH, 12000);
    
    if (duration == 0) {
        _telemetry.distanceCM = 999.0f; // Nothing detected in range
    } else {
        // Speed of sound = 343 m/s = 0.0343 cm/us. Divide by 2 for round trip.
        _telemetry.distanceCM = (duration * 0.0343f) / 2.0f;
    }
}
