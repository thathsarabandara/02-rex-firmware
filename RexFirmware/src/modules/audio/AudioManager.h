#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <Arduino.h>
#include <driver/i2s.h>
#include "../../../include/Config.h"

class AudioManager {
public:
    AudioManager();
    bool begin();
    
    // Play a sine-wave tone over the speaker
    void playTone(int frequency, int durationMs);
    
    // Read raw 32-bit audio samples from the microphone (INMP441)
    esp_err_t readMicrophone(void* destBuffer, size_t size, size_t* bytesRead);

private:
    bool _speakerInitialized;
    bool _micInitialized;
};

#endif // AUDIO_MANAGER_H
