/*
 * ╔══════════════════════════════════════════════════════════════╗
 * ║     Experiment 11: Multiple Sensor Monitoring (Zephyr RTOS)  ║
 * ║                                                              ║
 * ╠══════════════════════════════════════════════════════════════╣
 * ║  WIRING                                                      ║
 * ║                                                              ║
 * ║   Sensor               Pico GPIO    ADC Channel              ║
 * ║   ─────────            ─────────    ───────────              ║
 * ║   Light Sensor (LDR)   GP26         ADC0                     ║
 * ║   Temperature Sensor   GP27         ADC1                     ║
 * ║   Joystick X-axis      GP28         ADC2                     ║
 * ╚══════════════════════════════════════════════════════════════╝
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/adc.h>
#include <stdio.h>

#define STACK_SIZE 1024
#define PRIORITY   7

/* ADC Configuration */
#define ADC_NODE DT_NODELABEL(adc)
static const struct device *adc_dev = DEVICE_DT_GET(ADC_NODE);

#define ADC_RESOLUTION       12
#define ADC_GAIN             ADC_GAIN_1
#define ADC_REFERENCE        ADC_REF_INTERNAL
#define ADC_ACQUISITION_TIME ADC_ACQ_TIME_DEFAULT

#define LDR_CHANNEL  0
#define TEMP_CHANNEL 1
#define JOY_CHANNEL  2

/* Helper function to configure an ADC channel */
static void setup_adc_channel(uint8_t channel) {
	struct adc_channel_cfg channel_cfg = {
		.gain = ADC_GAIN,
		.reference = ADC_REFERENCE,
		.acquisition_time = ADC_ACQUISITION_TIME,
		.channel_id = channel,
		.differential = 0
	};
	adc_channel_setup(adc_dev, &channel_cfg);
}

/* Helper function to read from an ADC channel */
static int16_t read_adc(uint8_t channel) {
	int16_t sample_buffer[1];
	struct adc_sequence sequence = {
		.channels    = BIT(channel),
		.buffer      = sample_buffer,
		.buffer_size = sizeof(sample_buffer),
		.resolution  = ADC_RESOLUTION,
	};
	
	if (adc_read(adc_dev, &sequence) < 0) {
		return -1;
	}
	return sample_buffer[0];
}

/* ─────────────────────────────────────────────────────────────
 * Thread 1: Temperature Sensor (ADC1 / GP27)
 * Runs every 1000 ms.
 * ───────────────────────────────────────────────────────────── */
void temperature_task(void *a, void *b, void *c)
{
	setup_adc_channel(TEMP_CHANNEL);
	
	while (1) {
		int16_t raw_val = read_adc(TEMP_CHANNEL);
		
		/* Convert raw 12-bit ADC value (0-4095) to rough Temperature (C)
		 * Note: This is a placeholder conversion for demonstration.
		 * An LM35 or NTC thermistor requires specific math. 
		 * We map 0-4095 roughly to 20C-40C for demo purposes. */
		float temp_c = 20.0 + ((float)raw_val / 4095.0) * 20.0;
		
		printf("\nTemperature : %.1f C\n", (double)temp_c);
		
		k_sleep(K_MSEC(1000));
	}
}

/* ─────────────────────────────────────────────────────────────
 * Thread 2: Light Sensor / LDR (ADC0 / GP26)
 * Runs every 500 ms.
 * ───────────────────────────────────────────────────────────── */
void light_task(void *a, void *b, void *c)
{
	setup_adc_channel(LDR_CHANNEL);
	
	while (1) {
		int16_t raw_val = read_adc(LDR_CHANNEL);
		
		/* LDR values can just be raw to show intensity */
		printf("Light : %d\n", raw_val);
		
		k_sleep(K_MSEC(500));
	}
}

/* ─────────────────────────────────────────────────────────────
 * Thread 3: Joystick X-Axis (ADC2 / GP28)
 * Runs every 200 ms.
 * ───────────────────────────────────────────────────────────── */
void joystick_task(void *a, void *b, void *c)
{
	setup_adc_channel(JOY_CHANNEL);
	
	while (1) {
		int16_t raw_val = read_adc(JOY_CHANNEL);
		
		/* Joystick values (0-4095) */
		printf("Joystick : %d\n", raw_val);
		
		k_sleep(K_MSEC(200));
	}
}

/* ── Spawn the Threads ── */
K_THREAD_DEFINE(thread_temp, STACK_SIZE, temperature_task, NULL, NULL, NULL, PRIORITY, 0, 1000);
K_THREAD_DEFINE(thread_light, STACK_SIZE, light_task, NULL, NULL, NULL, PRIORITY, 0, 1000);
K_THREAD_DEFINE(thread_joy, STACK_SIZE, joystick_task, NULL, NULL, NULL, PRIORITY, 0, 1000);

/* ── Main Setup ── */
int main(void)
{
	if (!device_is_ready(adc_dev)) {
		printf("ADC device not ready!\n");
		return -1;
	}

	printf("\n==========================================\n");
	printf("Multiple Sensor Monitoring Started\n");
	printf("==========================================\n");

	/* Main thread exits, handing control fully to the RTOS scheduler */
	return 0;
}
