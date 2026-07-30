/*
 * ╔══════════════════════════════════════════════════════════════╗
 * ║     Experiment 9: Producer-Consumer Model (Potentiometer)    ║
 * ║                                                              ║
 * ╠══════════════════════════════════════════════════════════════╣
 * ║  WIRING                                                      ║
 * ║                                                              ║
 * ║   Component            Pico GPIO    ADC Channel              ║
 * ║   ─────────            ─────────    ───────────              ║
 * ║   Potentiometer        GP26         ADC0                     ║
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
#define POT_CHANNEL          0

/* ── Shared Variables & Mutex ── */
int16_t pot_value = 0;
bool new_data = false;
K_MUTEX_DEFINE(lock);

/* ─────────────────────────────────────────────────────────────
 * Task 1: Sampling Task (Producer)
 * ───────────────────────────────────────────────────────────── */
void sampling_task(void *a, void *b, void *c)
{
	int16_t sample_buffer[1];
	struct adc_sequence sequence = {
		.channels    = BIT(POT_CHANNEL),
		.buffer      = sample_buffer,
		.buffer_size = sizeof(sample_buffer),
		.resolution  = ADC_RESOLUTION,
	};

	while (1) {
		if (adc_read(adc_dev, &sequence) == 0) {
			int16_t val = sample_buffer[0];

			/* The Zephyr ADC provides a 12-bit value (0-4095). 
			   The Python code used a 16-bit value (0-65535). 
			   We will manually scale it to 16-bit so the math matches Python. */
			uint16_t scaled_val = (uint16_t)val << 4; /* Scale 12-bit to 16-bit roughly */

			/* Protect Shared Variables */
			k_mutex_lock(&lock, K_FOREVER);
			pot_value = scaled_val;
			new_data = true;
			k_mutex_unlock(&lock);
		}
		
		k_sleep(K_MSEC(100));
	}
}

/* ─────────────────────────────────────────────────────────────
 * Task 2: Processing Task (Consumer)
 * ───────────────────────────────────────────────────────────── */
void processing_task(void *a, void *b, void *c)
{
	while (1) {
		int32_t value = -1;

		/* Protect Shared Variables */
		k_mutex_lock(&lock, K_FOREVER);
		if (new_data) {
			value = pot_value;
			new_data = false;
		}
		k_mutex_unlock(&lock);

		if (value != -1) {
			int percentage = (value * 100) / 65535;

			printf("----------------------\n");
			printf("Raw Reading : %d\n", (int)value);
			printf("Percentage  : %d %%\n", percentage);
		}

		k_sleep(K_MSEC(100));
	}
}

/* ── Spawn the Threads ── */
K_THREAD_DEFINE(thread_sampling, STACK_SIZE, sampling_task, NULL, NULL, NULL, PRIORITY, 0, 1000);
K_THREAD_DEFINE(thread_processing, STACK_SIZE, processing_task, NULL, NULL, NULL, PRIORITY, 0, 1000);

/* ── Main Setup ── */
int main(void)
{
	/* Allow time for USB CDC ACM to enumerate on PC */
	k_sleep(K_MSEC(2000));

	if (!device_is_ready(adc_dev)) {
		printf("ADC device not ready!\n");
		return -1;
	}

	/* Initialize hardware synchronously before threads run */
	struct adc_channel_cfg channel_cfg = {
		.gain = ADC_GAIN,
		.reference = ADC_REFERENCE,
		.acquisition_time = ADC_ACQUISITION_TIME,
		.channel_id = POT_CHANNEL,
		.differential = 0
	};
	adc_channel_setup(adc_dev, &channel_cfg);

	printf("\n==========================================\n");
	printf("Producer-Consumer Model Started\n");
	printf("==========================================\n");

	return 0;
}
