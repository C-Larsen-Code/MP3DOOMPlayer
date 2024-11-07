#ifndef TONE_H_
#define TONE_H_

#include "sound.h"

// This component is a thin layer around the sound component.
// One cycle of a waveform is generated and then given to the
// sound component to play out cyclically until told to stop.
// Macros are provided for tone functions that are aliases
// of sound functions.

#define LOWEST_FREQ 20U // Hz

#define tone_stop() sound_stop()
#define tone_busy() sound_busy()
#define tone_set_volume(vol) sound_set_volume(vol)
#define tone_device(en) sound_device(en)

// Tone waveforms
typedef enum {SINE_T, SQUARE_T, TRIANGLE_T, SAW_T, LAST_T} tone_t;

// Initialize the tone driver. Must be called before using.
// May be called again to change sample rate.
// sample_hz: sample rate in Hz to playback tone.
// Return zero if successful, or non-zero otherwise.
int32_t tone_init(uint32_t sample_hz);

// Free resources used for tone generation (DAC, etc.).
// Return zero if successful, or non-zero otherwise.
int32_t tone_deinit(void);

// Start playing the specified tone.
// tone: one of the enumerated tone types.
// freq: frequency of the tone in Hz.
void tone_start(tone_t tone, uint32_t freq);

#endif // TONE_H_
