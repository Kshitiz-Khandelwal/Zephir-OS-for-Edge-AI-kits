/*
 * ╔══════════════════════════════════════════════════════════════╗
 * ║     Experiment 14: Data Logging (ADC & Mutex Flags)          ║
 * ║                                                              ║
 * ╠══════════════════════════════════════════════════════════════╣
 * ║  WIRING                                                      ║
 * ║                                                              ║
 * ║   Component            Pico GPIO    ADC Channel              ║
 * ║   ─────────            ─────────    ───────────              ║
 * ║   Light Sensor (LDR)   GP26         ADC0                     ║
 * ╚══════════════════════════════════════════════════════════════╝
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/adc.h>
#include <stdio.h>
#include <stdbool.h>

#define STACK_SIZE 1024
#define PRIORITY   7

/* ── ADC Configuration ── */
#define ADC_NODE DT_NODELABEL(adc)
static const struct device *adc_dev = DEVICE_DT_GET(ADC_NODE);

#define ADC_RESOLUTION       12
#define ADC_GAIN             ADC_GAIN_1
#define ADC_REFERENCE        ADC_REF_INTERNAL
#define ADC_ACQUISITION_TIME ADC_ACQ_TIME_DEFAULT
#define LDR_CHANNEL          0

/* ── Shared Variables & Mutex ── */
int16_t latest_reading = 0;
bool new_data = false;
K_MUTEX_DEFINE(lock);

/* ─────────────────────────────────────────────────────────────
 * Task 1: Logger Task
 * Reads ADC every 1 second and sets the flag.
 * ───────────────────────────────────────────────────────────── */
void logger_task(void *a, void *b, void *c)
{
	/* Setup ADC Channel */
	struct adc_channel_cfg channel_cfg = {
		.gain = ADC_GAIN,
		.reference = ADC_REFERENCE,
		.acquisition_time = ADC_ACQUISITION_TIME,
		.channel_id = LDR_CHANNEL,
		.differential = 0
	};
	adc_channel_setup(adc_dev, &channel_cfg);

	int16_t sample_buffer[1];
	struct adc_sequence sequence = {
		.channels    = BIT(LDR_CHANNEL),
		.buffer      = sample_buffer,
		.buffer_size = sizeof(sample_buffer),
		.resolution  = ADC_RESOLUTION,
	};

	while (1) {
		if (adc_read(adc_dev, &sequence) == 0) {
			int16_t value = sample_buffer[0];

			/* Protect Shared Variables */
			k_mutex_lock(&lock, K_FOREVER);
			latest_reading = value;
			new_data = true;
			printf("Logged Reading : %d\n", value);
			k_mutex_unlock(&lock);
		}
		
		k_sleep(K_MSEC(1000));
	}
}

/* ─────────────────────────────────────────────────────────────
 * Task 2: Display Task
 * Checks if new data is available every 100ms.
 * ───────────────────────────────────────────────────────────── */
void display_task(void *a, void *b, void *c)
{
	while (1) {
		/* Protect Shared Variables */
		k_mutex_lock(&lock, K_FOREVER);
		if (new_data) {
			printf("Latest Reading : %d\n", latest_reading);
			new_data = false;
		}
		k_mutex_unlock(&lock);

		k_sleep(K_MSEC(100));
	}
}

/* ── Spawn the Threads ── */
K_THREAD_DEFINE(thread_logger, STACK_SIZE, logger_task, NULL, NULL, NULL, PRIORITY, 0, 100);
K_THREAD_DEFINE(thread_display, STACK_SIZE, display_task, NULL, NULL, NULL, PRIORITY, 0, 100);

/* ── Main Setup ── */
int main(void)
{
	if (!device_is_ready(adc_dev)) {
		printf("ADC device not ready!\n");
		return -1;
	}

	printf("\n==========================================\n");
	printf("Data Logging Started\n");
	printf("==========================================\n");

	return 0;
}
