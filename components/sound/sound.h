#ifndef SOUND_H_
#define SOUND_H_

#include <stdbool.h>
#include <stdint.h>

#define MAX_VOL 100U

// Initialize the sound driver. Must be called before using sound.
// May be called again to change sample rate.
// sample_hz: sample rate in Hz to playback audio.
// Return zero if successful, or non-zero otherwise.
int32_t sound_init(uint32_t sample_hz);

// Free resources used for sound (DAC, etc.).
// Return zero if successful, or non-zero otherwise.
int32_t sound_deinit(void);

// Start playing the sound immediately. Play the audio buffer once.
// audio: a pointer to an array of unsigned audio data.
// size: the size of the array in bytes.
// wait: if true, block until done playing, otherwise return straight away.
void sound_start(const void *audio, uint32_t size, bool wait);

// Cyclically play samples from audio buffer until sound_stop() is called.
// audio: a pointer to an array of unsigned audio data.
// size: the size of the array in bytes.
void sound_cyclic(const void *audio, uint32_t size);

// Return true if sound playing, otherwise return false.
bool sound_busy(void);

// Stop playing the sound.
void sound_stop(void);

// Set the volume.
// volume: 0-100% as an integer value.
void sound_set_volume(uint32_t vol);

// Enable or disable the sound output device.
// enable: if true, enable sound, otherwise disable.
void sound_device(bool enable);

#endif // SOUND_H_
