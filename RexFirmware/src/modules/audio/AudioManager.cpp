#include "AudioManager.h"
#include <math.h>

AudioManager::AudioManager() 
    : _speakerInitialized(false), _micInitialized(false) {}

bool AudioManager::begin() {
    // 1. Initialize Speaker (MAX98357) on I2S_NUM_1
    i2s_config_t speaker_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };
    
    i2s_pin_config_t speaker_pins = {
        .bck_io_num = MAX98357_BCLK,
        .ws_io_num = MAX98357_LRC,
        .data_out_num = MAX98357_DIN,
        .data_in_num = I2S_PIN_NO_CHANGE
    };
    
    if (i2s_driver_install(I2S_NUM_1, &speaker_config, 0, NULL) == ESP_OK) {
        if (i2s_set_pin(I2S_NUM_1, &speaker_pins) == ESP_OK) {
            Serial.println(" -> AudioManager: MAX98357 Speaker (I2S) OK.");
            _speakerInitialized = true;
            
            // Play power-up tone: ascending C major chord (C5 -> E5 -> G5 -> C6)
            playTone(523, 120); // C5
            delay(20);
            playTone(659, 120); // E5
            delay(20);
            playTone(784, 120); // G5
            delay(20);
            playTone(1047, 250); // C6
        } else {
            Serial.println(" -> AudioManager: MAX98357 Speaker (I2S) Pin Config FAILED.");
        }
    } else {
        Serial.println(" -> AudioManager: MAX98357 Speaker (I2S) Driver FAILED.");
    }

    // 2. Initialize Microphone (INMP441) on I2S_NUM_0
    i2s_config_t mic_config = {
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
    
    i2s_pin_config_t mic_pins = {
        .bck_io_num = INMP441_SCK,
        .ws_io_num = INMP441_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = INMP441_SD
    };
    
    if (i2s_driver_install(I2S_NUM_0, &mic_config, 0, NULL) == ESP_OK) {
        if (i2s_set_pin(I2S_NUM_0, &mic_pins) == ESP_OK) {
            Serial.println(" -> AudioManager: INMP441 Microphone (I2S) OK.");
            _micInitialized = true;
        } else {
            Serial.println(" -> AudioManager: INMP441 Microphone (I2S) Pin Config FAILED.");
        }
    } else {
        Serial.println(" -> AudioManager: INMP441 Microphone (I2S) Driver FAILED.");
    }

    return (_speakerInitialized && _micInitialized);
}

void AudioManager::playTone(int frequency, int durationMs) {
    if (!_speakerInitialized) return;

    int sampleRate = 44100;
    int numSamples = (sampleRate * durationMs) / 1000;
    const int chunkSize = 256;
    int16_t buffer[chunkSize * 2]; // 16-bit stereo (L + R)
    
    int sampleCount = 0;
    while (sampleCount < numSamples) {
        int toWrite = numSamples - sampleCount;
        if (toWrite > chunkSize) toWrite = chunkSize;
        
        for (int i = 0; i < toWrite; i++) {
            double t = (double)(sampleCount + i) / sampleRate;
            double val = sin(2.0 * PI * frequency * t) * 5000.0; // Moderate volume amplitude
            int16_t sample = (int16_t)val;
            buffer[i * 2] = sample;     // Left channel
            buffer[i * 2 + 1] = sample; // Right channel
        }
        
        size_t bytesWritten;
        i2s_write(I2S_NUM_1, buffer, toWrite * 2 * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
        sampleCount += toWrite;
    }
    
    // Write a small buffer of silence to clear I2S pipeline and prevent pop/buzz
    memset(buffer, 0, sizeof(buffer));
    size_t bytesWritten;
    i2s_write(I2S_NUM_1, buffer, sizeof(buffer), &bytesWritten, portMAX_DELAY);
}

esp_err_t AudioManager::readMicrophone(void* destBuffer, size_t size, size_t* bytesRead) {
    if (!_micInitialized) return ESP_ERR_INVALID_STATE;
    return i2s_read(I2S_NUM_0, destBuffer, size, bytesRead, portMAX_DELAY);
}
