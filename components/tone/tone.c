#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "tone.h"


// Global Variables
#define SINE_T 0
#define SQUARE_T 1
#define TRIANGLE_T 2
#define SAW_T 3
#define BIAS_LEVEL 0x80
#define MAX_VALUE 0xFF
#define BIAS_LEVEL_LOW 0x7F
#define QUARTER_THREE 3
#define QUARTER_OF_PERIOD 4

uint8_t *buffer = NULL;
uint32_t sample_rate = 0;
uint32_t buffer_size = 0;


// Initialize the tone driver. Must be called before using.
// May be called again to change sample rate.
// sample_hz: sample rate in Hz to playback tone.
// Return zero if successful, or non-zero otherwise.
int32_t tone_init(uint32_t sample_hz) {
    // Check the sample rate parameter to make sure it is at least twice the lowest Nyquist frequency.
    if (sample_hz < 2 * LOWEST_FREQ) {
        return -1;
    }

    
    // Call sound_init() from the sound component (provided in the starting code).
    sound_init(sample_hz);
    buffer_size = sample_hz / LOWEST_FREQ;
    sample_rate = sample_hz;
    buffer = malloc(sizeof(uint8_t) * buffer_size);

    return 0;
    
}

// Free resources used for tone generation (DAC, etc.).
// Return zero if successful, or non-zero otherwise.
int32_t tone_deinit(void) {
    // Free the waveform buffer allocated during init.
    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }

    // Call sound_deinit().
    sound_deinit();

    return 0;
}

// Start playing the specified tone.
// tone: one of the enumerated tone types.
// freq: frequency of the tone in Hz.
void tone_start(tone_t tone, uint32_t freq) {
    // Check the parameters to see if they are within bounds.
    if (freq < LOWEST_FREQ || freq > sample_rate / 2 || (tone > LAST_T) ){
        return;
    }

    // Calculate the number of samples in one period for the specified frequency (DAC sample rate / waveform frequency).
    uint32_t samples_per_period = sample_rate / freq;

    // Generate one cycle of the specified waveform into the allocated buffer. The element type of the buffer should be uint8_t.
    for (uint32_t i = 0; i < samples_per_period; i++){
    switch (tone) {
        case SINE_T:
            // Generate a sine wave
            buffer[i] = (uint8_t)(BIAS_LEVEL + BIAS_LEVEL_LOW * sinf(2 * M_PI * i / samples_per_period));
            break;
        case SQUARE_T:
            // Generate a square wave
                buffer[i] = (i < samples_per_period / 2) ? MAX_VALUE : 0;
            break;
        case TRIANGLE_T:
            // Generate a triangle wave
            if (i < samples_per_period / QUARTER_OF_PERIOD) {
                    // First quarter: rising from bias to max
                    buffer[i] = (uint8_t)(BIAS_LEVEL + (QUARTER_OF_PERIOD * BIAS_LEVEL_LOW * i) / samples_per_period);
                } else if (i < QUARTER_THREE * samples_per_period / QUARTER_OF_PERIOD) {
                    // Second and third quarters: falling from max to min
                    buffer[i] = (uint8_t)(BIAS_LEVEL + BIAS_LEVEL_LOW - (QUARTER_OF_PERIOD * BIAS_LEVEL_LOW * (i - samples_per_period / QUARTER_OF_PERIOD)) / samples_per_period);
                } else {
                    // Last quarter: rising from min to bias
                    buffer[i] = (uint8_t)(BIAS_LEVEL - BIAS_LEVEL_LOW + (QUARTER_OF_PERIOD * BIAS_LEVEL_LOW * (i - QUARTER_THREE * samples_per_period / QUARTER_OF_PERIOD)) / samples_per_period);
                }
                break;
        case SAW_T:
            // Generate a sawtooth wave
                buffer[i] = (uint8_t)(BIAS_LEVEL + BIAS_LEVEL_LOW * (2.0f * i / (float)samples_per_period));
            break;
        default:
            return;
    }
}

    // Call sound_cyclic() to play the generated waveform.
    sound_cyclic(buffer, samples_per_period);
}

