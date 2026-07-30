/*
 * ╔══════════════════════════════════════════════════════════════╗
 * ║     Advanced Sensor Display (ADC, TM1637, Mutex, 4 Threads)  ║
 * ║                                                              ║
 * ╠══════════════════════════════════════════════════════════════╣
 * ║  WIRING                                                      ║
 * ║                                                              ║
 * ║   Component            Pico GPIO    ADC Channel              ║
 * ║   ─────────            ─────────    ───────────              ║
 * ║   Light Sensor (LDR)   GP26         ADC0                     ║
 * ║   Temperature Sensor   GP27         ADC1                     ║
 * ║   Joystick X-axis      GP28         ADC2                     ║
 * ║                                                              ║
 * ║   TM1637 CLK           GP3                                   ║
 * ║   TM1637 DIO           GP2                                   ║
 * ║   TM1637 VCC           5V (VBUS)                             ║
 * ║   TM1637 GND           GND                                   ║
 * ╚══════════════════════════════════════════════════════════════╝
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <stdio.h>

#define STACK_SIZE 1024
#define PRIORITY   7

/* ── ADC Configuration ── */
#define ADC_NODE DT_NODELABEL(adc)
static const struct device *adc_dev = DEVICE_DT_GET(ADC_NODE);

#define ADC_RESOLUTION       12
#define ADC_GAIN             ADC_GAIN_1
#define ADC_REFERENCE        ADC_REF_INTERNAL
#define ADC_ACQUISITION_TIME ADC_ACQ_TIME_DEFAULT

#define LDR_CHANNEL  0
#define TEMP_CHANNEL 1
#define JOY_CHANNEL  2

/* ── TM1637 Configuration ── */
#define CLK_PIN 3
#define DIO_PIN 2
static const struct device *gpio_dev;

/* ── Shared Variables & Mutex ── */
struct sensor_data_t {
	float temperature;
	int16_t light;
	int16_t joystick;
};

struct sensor_data_t shared_sensors = {0};
K_MUTEX_DEFINE(sensor_mutex);

/* ─────────────────────────────────────────────────────────────
 * TM1637 Bit-Bang Driver 
 * ───────────────────────────────────────────────────────────── */
static uint8_t get_char_code(char c) {
	switch (c) {
		case '0': return 0x3F; case '1': return 0x06;
		case '2': return 0x5B; case '3': return 0x4F;
		case '4': return 0x66; case '5': return 0x6D;
		case '6': return 0x7D; case '7': return 0x07;
		case '8': return 0x7F; case '9': return 0x6F;
		case 'C': return 0x39; case 'L': return 0x38;
		case 'J': return 0x1E; case 't': return 0x78;
		case ' ': return 0x00; default:  return 0x00;
	}
}

static void start(void) {
	gpio_pin_set(gpio_dev, DIO_PIN, 1);
	gpio_pin_set(gpio_dev, CLK_PIN, 1);
	gpio_pin_set(gpio_dev, DIO_PIN, 0);
	gpio_pin_set(gpio_dev, CLK_PIN, 0);
}

static void stop(void) {
	gpio_pin_set(gpio_dev, CLK_PIN, 0);
	gpio_pin_set(gpio_dev, DIO_PIN, 0);
	gpio_pin_set(gpio_dev, CLK_PIN, 1);
	gpio_pin_set(gpio_dev, DIO_PIN, 1);
}

static void write_byte(uint8_t b) {
	gpio_pin_configure(gpio_dev, DIO_PIN, GPIO_OUTPUT | GPIO_OPEN_DRAIN);
	for (int i = 0; i < 8; i++) {
		gpio_pin_set(gpio_dev, CLK_PIN, 0);
		gpio_pin_set(gpio_dev, DIO_PIN, b & 1);
		k_busy_wait(20);
		gpio_pin_set(gpio_dev, CLK_PIN, 1);
		k_busy_wait(20);
		b >>= 1;
	}
	gpio_pin_set(gpio_dev, CLK_PIN, 0);
	gpio_pin_configure(gpio_dev, DIO_PIN, GPIO_INPUT | GPIO_PULL_UP);
	gpio_pin_set(gpio_dev, CLK_PIN, 1);
	k_busy_wait(20);
	gpio_pin_set(gpio_dev, CLK_PIN, 0);
	gpio_pin_configure(gpio_dev, DIO_PIN, GPIO_OUTPUT | GPIO_OPEN_DRAIN);
}

static void tm1637_display(const char *str) {
	start();
	write_byte(0x40);
	stop();

	start();
	write_byte(0xC0);
	for (int i = 0; i < 4; i++) { 
		write_byte(get_char_code(str[i])); 
	}
	stop();

	start();
	write_byte(0x8A);
	stop();
}

/* ─────────────────────────────────────────────────────────────
 * ADC Helper Functions
 * ───────────────────────────────────────────────────────────── */
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
	while (1) {
		int16_t raw_val = read_adc(TEMP_CHANNEL);
		float temp_c = 20.0 + ((float)raw_val / 4095.0) * 20.0; /* Mock conversion */
		
		/* Protect shared resource update */
		k_mutex_lock(&sensor_mutex, K_FOREVER);
		shared_sensors.temperature = temp_c;
		k_mutex_unlock(&sensor_mutex);
		
		printf("Temperature : %.1f C\n", (double)temp_c);
		k_sleep(K_MSEC(1000));
	}
}

/* ─────────────────────────────────────────────────────────────
 * Thread 2: Light Sensor / LDR (ADC0 / GP26)
 * Runs every 500 ms.
 * ───────────────────────────────────────────────────────────── */
void light_task(void *a, void *b, void *c)
{
	while (1) {
		int16_t raw_val = read_adc(LDR_CHANNEL);
		
		/* Protect shared resource update */
		k_mutex_lock(&sensor_mutex, K_FOREVER);
		shared_sensors.light = raw_val;
		k_mutex_unlock(&sensor_mutex);
		
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
	while (1) {
		int16_t raw_val = read_adc(JOY_CHANNEL);
		
		/* Protect shared resource update */
		k_mutex_lock(&sensor_mutex, K_FOREVER);
		shared_sensors.joystick = raw_val;
		k_mutex_unlock(&sensor_mutex);
		
		printf("Joystick : %d\n", raw_val);
		k_sleep(K_MSEC(200));
	}
}

/* ─────────────────────────────────────────────────────────────
 * Thread 4: Display Task (TM1637)
 * Runs every 100 ms, updates display every 2000 ms.
 * ───────────────────────────────────────────────────────────── */
void display_task(void *a, void *b, void *c)
{
	int display_mode = 0; /* 0 = Temp, 1 = Light, 2 = Joystick */
	char display_str[5];

	while (1) {
		/* Protect shared resource read */
		k_mutex_lock(&sensor_mutex, K_FOREVER);
		float t = shared_sensors.temperature;
		int16_t l = shared_sensors.light;
		int16_t j = shared_sensors.joystick;
		k_mutex_unlock(&sensor_mutex);
		
		if (display_mode == 0) {
			snprintf(display_str, sizeof(display_str), "t %2d", (int)t);
		} else if (display_mode == 1) {
			snprintf(display_str, sizeof(display_str), "L%3d", l / 10); /* Scale for 3 digits */
		} else if (display_mode == 2) {
			snprintf(display_str, sizeof(display_str), "J%3d", j / 10); /* Scale for 3 digits */
		}

		tm1637_display(display_str);
		
		display_mode++;
		if (display_mode > 2) display_mode = 0;
		
		k_sleep(K_MSEC(2000));
	}
}

/* ── Spawn the Threads ── */
/*
 * Do not start these threads until main() has configured the ADC and GPIO.
 * Starting them with a fixed delay races main() and can make the display task
 * access gpio_dev while it is still NULL.
 */
K_THREAD_STACK_DEFINE(temp_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(light_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(joy_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(disp_stack, STACK_SIZE);

struct k_thread thread_temp;
struct k_thread thread_light;
struct k_thread thread_joy;
struct k_thread thread_disp;

/* ── Main Setup ── */
int main(void)
{
	/* Allow time for USB CDC ACM to enumerate on PC */
	k_sleep(K_MSEC(2000));

	if (!device_is_ready(adc_dev)) {
		printf("ADC device not ready!\n");
		return -1;
	}

	setup_adc_channel(TEMP_CHANNEL);
	setup_adc_channel(LDR_CHANNEL);
	setup_adc_channel(JOY_CHANNEL);

	gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));
	if (!device_is_ready(gpio_dev)) {
		return -1;
	}

	gpio_pin_configure(gpio_dev, CLK_PIN, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure(gpio_dev, DIO_PIN, GPIO_OUTPUT | GPIO_OPEN_DRAIN);
	gpio_pin_set(gpio_dev, CLK_PIN, 1);
	gpio_pin_set(gpio_dev, DIO_PIN, 1);

	printf("\n==========================================\n");
	printf("Advanced Sensor Display Started\n");
	printf("==========================================\n");

	/* Hardware smoke test: all segments must light for two seconds. */
	tm1637_display("8888");
	k_sleep(K_MSEC(2000));

	k_thread_create(&thread_temp, temp_stack, K_THREAD_STACK_SIZEOF(temp_stack), temperature_task, NULL, NULL, NULL, PRIORITY, 0, K_NO_WAIT);
	k_thread_create(&thread_light, light_stack, K_THREAD_STACK_SIZEOF(light_stack), light_task, NULL, NULL, NULL, PRIORITY, 0, K_NO_WAIT);
	k_thread_create(&thread_joy, joy_stack, K_THREAD_STACK_SIZEOF(joy_stack), joystick_task, NULL, NULL, NULL, PRIORITY, 0, K_NO_WAIT);
	k_thread_create(&thread_disp, disp_stack, K_THREAD_STACK_SIZEOF(disp_stack), display_task, NULL, NULL, NULL, PRIORITY, 0, K_NO_WAIT);

	return 0;
}
