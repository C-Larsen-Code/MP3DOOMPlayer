// https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/dac.html
// https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/gptimer.html

#include <math.h> // roundf

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "driver/dac_oneshot.h"

#include "hw.h"
#include "sound.h"

// Make audio buffer extern for testing
#ifdef EXTERN_BUF
#define scope
#else
#define scope static
#endif

#define SOUND_A  HW_SND_A  // Audio output
#define SOUND_EN HW_SND_EN // Sound enable, active high

#define GPTIMER_RESOLUTION_HZ 1000000

#define SOUND_VOLUME_DEFAULT 50
#define SILENCE 0x80U
#define POLL_DELAY 10
#define PERCENT 100U

static const char *TAG = "sound";

// Critical section protected variables
static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;
scope  const uint8_t *abase;
scope  volatile uint32_t asize;
static volatile uint32_t aidx;
static volatile bool     cyclic;

// Other global variables
static dac_oneshot_handle_t dac_handle;
static gptimer_handle_t dac_timer;
static volatile bool device_en;
static volatile uint32_t volume;
static volatile uint8_t bias; // to prevent popping at end when vol low.


// DAC timer ISR callback
static bool IRAM_ATTR dac_timer_isr(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
	// TODO: why does critical section cause abort?
	// portENTER_CRITICAL_ISR(&spinlock);
	if (aidx < asize) {
		uint32_t idx = aidx;
		aidx =  cyclic ? (aidx + 1) % asize : aidx + 1;
		// portEXIT_CRITICAL_ISR(&spinlock);
		dac_oneshot_output_voltage(dac_handle, abase[idx]*volume/PERCENT + bias);
	} else if (aidx == asize) {
		aidx++;
		// portEXIT_CRITICAL_ISR(&spinlock);
		dac_oneshot_output_voltage(dac_handle, SILENCE);
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

	// if the first time called, configure pins
	if (dac_handle == NULL) {
		/* * * * * * * * * * Enable Pin Config * * * * * * * * * */
		gpio_config_t io_conf = {}; // zero-initialize the config structure
		io_conf.intr_type = GPIO_INTR_DISABLE;
		io_conf.mode = GPIO_MODE_OUTPUT;
		// bit mask of the pins that you want to set, e.g. GPIO25
		io_conf.pin_bit_mask = 1ULL<<SOUND_EN;
		io_conf.pull_down_en = 0;
		io_conf.pull_up_en = 0;
		gpio_config(&io_conf);
		gpio_set_level(SOUND_EN, device_en = true);

		/* * * * * * * * * * Audio Pin and DAC Config * * * * * * * * * */
		if (SOUND_A != 26) {
			ESP_LOGE(TAG, "driver only supports internal DAC on GPIO26");
			return 1;
		}
		dac_oneshot_config_t one_cfg = {
			.chan_id = DAC_CHAN_1, // GPIO26 on ESP32
		};
		ESP_ERROR_CHECK(dac_oneshot_new_channel(&one_cfg, &dac_handle));
	}

	// if the first time called, create and configure timer
	if (dac_timer == NULL) {
		// TODO: how to setup timer interrupt on CPU 1
		// Create a new timer instance
		gptimer_config_t dac_timer_config = {
			.clk_src = GPTIMER_CLK_SRC_DEFAULT,
			.direction = GPTIMER_COUNT_UP,
			.resolution_hz = GPTIMER_RESOLUTION_HZ,
		};
		ESP_ERROR_CHECK(gptimer_new_timer(&dac_timer_config, &dac_timer));

		// Register event callback
		gptimer_event_callbacks_t dac_cbs = {
			.on_alarm = dac_timer_isr, // register user callback
		};
		ESP_ERROR_CHECK(gptimer_register_event_callbacks(dac_timer, &dac_cbs, NULL));

		// Set up alarm action
		gptimer_alarm_config_t dac_alarm_config = {
			.reload_count = 0, // counter will reload with 0 on alarm event
			.alarm_count = roundf((float)GPTIMER_RESOLUTION_HZ/sample_hz),
			.flags.auto_reload_on_alarm = true, // enable auto-reload
		};
		ESP_LOGI(TAG, "alarm_count: %llu", dac_alarm_config.alarm_count);
		ESP_ERROR_CHECK(gptimer_set_alarm_action(dac_timer, &dac_alarm_config));

		// Enable and start timer
		ESP_LOGI(TAG, "Start DAC timer");
		ESP_ERROR_CHECK(gptimer_enable(dac_timer));
		ESP_ERROR_CHECK(gptimer_start(dac_timer));
	} else {
		// Set up alarm action
		gptimer_alarm_config_t dac_alarm_config = {
			.reload_count = 0, // counter will reload with 0 on alarm event
			.alarm_count = roundf((float)GPTIMER_RESOLUTION_HZ/sample_hz),
			.flags.auto_reload_on_alarm = true, // enable auto-reload
		};
		ESP_LOGI(TAG, "alarm_count: %llu", dac_alarm_config.alarm_count);
		ESP_ERROR_CHECK(gptimer_set_alarm_action(dac_timer, &dac_alarm_config));
	}
	return 0;
}

// Free resources used for sound (DAC, etc.).
// Return zero if successful, or non-zero otherwise.
int32_t sound_deinit(void)
{
	ESP_LOGI(TAG, "Stop DAC Timer");
	ESP_ERROR_CHECK(gptimer_stop(dac_timer));
	ESP_ERROR_CHECK(gptimer_disable(dac_timer));
	ESP_ERROR_CHECK(gptimer_del_timer(dac_timer));
	ESP_ERROR_CHECK(dac_oneshot_del_channel(dac_handle));
	dac_handle = NULL;
	dac_timer = NULL;
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
