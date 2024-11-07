// https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/dac.html

#include <string.h> // memset

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/dac_continuous.h"
#include "driver/gpio.h"

#include "hw.h"
#include "sound.h"

#define SOUND_A  HW_SND_A  // Audio output
#define SOUND_EN HW_SND_EN // Sound enable, active high

#define DAC_DESC_NUM 8 // Number of DAC descriptors
#define DAC_BUF_SZ 128 // DAC buffer size in bytes
// was able to play audio at 48kHz with buf size of  8 and 8 desc, async.
// was able to play audio at 48kHz with buf size of 64 and 8 desc, sync w/ vol control.

#define SOUND_VOLUME_DEFAULT 50
#define SILENCE 0x80U
#define POLL_DELAY 10
#define PERCENT 100U

static const char *TAG = "sound";

// Critical section protected variables
static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;
static const uint8_t *abase;
static volatile uint32_t asize;
static volatile uint32_t aidx;
static volatile bool     cyclic;
static volatile uint32_t dcnt;

// Other global variables
static dac_continuous_handle_t dac_handle;
static volatile bool device_en;
static volatile uint32_t volume;
static volatile uint8_t bias; // to prevent popping at end when vol low.


static bool IRAM_ATTR dac_convert_callback(dac_continuous_handle_t handle,
	const dac_event_data_t *event, void *user_data)
{
#if CONFIG_DAC_DMA_AUTO_16BIT_ALIGN
	uint8_t buf[event->buf_size/2];
#else
	uint8_t buf[event->buf_size];
#endif
	// size_t load_bytes = 0;
	portENTER_CRITICAL_ISR(&spinlock);
	if (aidx < asize) {
		uint32_t idx = aidx;
		uint32_t size = (!cyclic && asize < sizeof(buf)) ? asize : sizeof(buf);
		aidx =  cyclic ? (aidx + size) % asize : aidx + size;
		portEXIT_CRITICAL_ISR(&spinlock);
		uint32_t i;
		for (i = 0; i < size; i++) buf[i] = abase[(idx+i)%asize]*volume/PERCENT + bias;
		while (i < sizeof(buf)) buf[i++] = SILENCE; // pad to block size with silence
		dac_continuous_write_asynchronously(handle,
			event->buf, event->buf_size,
			buf, sizeof(buf), NULL /*&load_bytes*/);
			// error if load_bytes != sizeof(buf)
	} else if (dcnt) {
		dcnt--;
		portEXIT_CRITICAL_ISR(&spinlock);
		memset(buf, SILENCE, sizeof(buf));
		dac_continuous_write_asynchronously(handle,
			event->buf, event->buf_size,
			buf, sizeof(buf), NULL /*&load_bytes*/);
	} else {
		portEXIT_CRITICAL_ISR(&spinlock);
	}
	return false; // no high priority task awoken
}


// Initialize the sound driver. Must be called before using sound.
// May be called again to change sample rate.
// sample_hz: sample rate in Hz to playback audio.
// Return zero if successful, or non-zero otherwise.
int32_t sound_init(uint32_t sample_hz)
{
	sound_set_volume(SOUND_VOLUME_DEFAULT);
	
	/* * * * * * * * * * GPIO25 Pin Config * * * * * * * * * */
	// if the first time called, configure GPIO25 as output
	if (dac_handle == NULL) {
		gpio_config_t io_conf = {}; // zero-initialize the config structure
		io_conf.intr_type = GPIO_INTR_DISABLE;
		io_conf.mode = GPIO_MODE_OUTPUT;
		// bit mask of the pins that you want to set, e.g. GPIO25
		io_conf.pin_bit_mask = 1ULL<<SOUND_EN;
		io_conf.pull_down_en = 0;
		io_conf.pull_up_en = 0;
		gpio_config(&io_conf);
		gpio_set_level(SOUND_EN, device_en = true);
	}

	// If sound_init called previously, disable & delete prior channel
	if (dac_handle != NULL) sound_deinit();

	/* * * * * * * * * * GPIO26 Sound Config * * * * * * * * * */
	if (SOUND_A != 26) {
		ESP_LOGE(TAG, "driver only supports internal DAC on GPIO26");
		return 1;
	}
	dac_continuous_config_t cont_cfg = {
		.chan_mask = DAC_CHANNEL_MASK_CH1, // GPIO26 only
		.desc_num = DAC_DESC_NUM,
		.buf_size = DAC_BUF_SZ,
		.freq_hz = sample_hz,
		.offset = 0,
		.clk_src = DAC_DIGI_CLK_SRC_DEFAULT,
		// .clk_src = DAC_DIGI_CLK_SRC_APLL, // APLL source caused noise
		/* Assume the data in buffer is 'A B C D E F'
		 * DAC_CHANNEL_MODE_SIMUL:
		 *      - channel 0: A B C D E F
		 *      - channel 1: A B C D E F
		 * DAC_CHANNEL_MODE_ALTER:
		 *      - channel 0: A C E
		 *      - channel 1: B D F
		 */
		.chan_mode = DAC_CHANNEL_MODE_SIMUL,
	};

	// Allocate continuous channels
	ESP_ERROR_CHECK(dac_continuous_new_channels(&cont_cfg, &dac_handle));
	dac_event_callbacks_t cbs = {
		.on_convert_done = dac_convert_callback, // provides a free DMA buffer
		.on_stop = NULL, // indicates when all data has been sent
	};
	// Register the callback for asynchronous writing
	ESP_ERROR_CHECK(dac_continuous_register_event_callback(dac_handle, &cbs, NULL));
	// Enable the continuous channels
	ESP_ERROR_CHECK(dac_continuous_enable(dac_handle));
	ESP_LOGI(TAG, "Start async audio DMA");
	ESP_ERROR_CHECK(dac_continuous_start_async_writing(dac_handle));
	return 0;
}

// Free resources used for sound (DAC, etc.).
// Return zero if successful, or non-zero otherwise.
int32_t sound_deinit(void)
{
	ESP_LOGI(TAG, "Stop async audio DMA");
	ESP_ERROR_CHECK(dac_continuous_stop_async_writing(dac_handle));
	ESP_ERROR_CHECK(dac_continuous_disable(dac_handle));
	dac_event_callbacks_t cbs = {}; // is this necessary before delete?
	ESP_ERROR_CHECK(dac_continuous_register_event_callback(dac_handle, &cbs, NULL));
	ESP_ERROR_CHECK(dac_continuous_del_channels(dac_handle));
	dac_handle = NULL;
	return 0;
}

// Start playing the sound immediately. Play the audio buffer once.
// audio: a pointer to an array of unsigned audio data.
// size: the size of the array in bytes.
// wait: if true, block until done playing, otherwise return straight away.
void sound_start(const void *audio, uint32_t size, bool wait)
{
	portENTER_CRITICAL(&spinlock);
	abase = audio;
	asize = size;
	aidx = 0;
	cyclic = false;
	dcnt = DAC_DESC_NUM;
	portEXIT_CRITICAL(&spinlock);
	while (wait && aidx < asize)
		vTaskDelay(pdMS_TO_TICKS(POLL_DELAY));
}

// Cyclically play samples from audio buffer until sound_stop() is called.
// audio: a pointer to an array of unsigned audio data.
// size: the size of the array in bytes.
void sound_cyclic(const void *audio, uint32_t size)
{
	portENTER_CRITICAL(&spinlock);
	abase = audio;
	asize = size;
	aidx = 0;
	cyclic = true;
	dcnt = DAC_DESC_NUM;
	portEXIT_CRITICAL(&spinlock);
}

// Return true if sound playing, otherwise return false.
bool sound_busy(void)
{
	return aidx < asize;
}

// Stop playing the sound.
void sound_stop(void)
{
	portENTER_CRITICAL(&spinlock);
	aidx = asize;
	portEXIT_CRITICAL(&spinlock);
}

// Set the volume.
// volume: 0-100% as an integer value.
void sound_set_volume(uint32_t vol)
{
	volume = vol;
	bias = SILENCE - (SILENCE * vol / PERCENT);
}

// Enable or disable the sound output device.
// enable: if true, enable sound, otherwise disable.
void sound_device(bool enable)
{
	gpio_set_level(SOUND_EN, device_en = enable);
}

// NOTES:
// * Switching back and forth between sync and async crashes with WDT timeout
// in ISR. The crash happens when async follows sync.
// * Calling dac_continuous_start_async_writing() starts sending data out the
// DAC from the DMA buffers. Source code shows this function also clears the
// DMA buffers (to zero) before starting the DMA so you don't have lots of
// noise. If a callback is registered, the routine is called under interrupt
// each time a DMA buffer is finished being transferred.
// * The dac_event_data_t buffer appears to provide the full size of DMA buffer
// as specified in cont_cfg.
// Floating point is not allowed in an ISR unless config FREERTOS_FPU_IN_ISR
// is set.

