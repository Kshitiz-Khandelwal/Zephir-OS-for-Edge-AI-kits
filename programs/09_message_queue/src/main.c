/*
 * ╔══════════════════════════════════════════════════════════════╗
 * ║     Message Queues in Zephyr RTOS (Data Passing)             ║
 * ║                                                              ║
 * ╠══════════════════════════════════════════════════════════════╣
 * ║  WIRING                                                      ║
 * ║                                                              ║
 * ║   Component    Pin    Raspberry Pi Pico                      ║
 * ║   ─────────    ───    ─────────────────                      ║
 * ║   TM1637 CLK   CLK  → GP16                                   ║
 * ║   TM1637 DIO   DIO  → GP17                                   ║
 * ║   TM1637 VCC   VCC  → 5V (VBUS)                              ║
 * ║   TM1637 GND   GND  → GND                                    ║
 * ╚══════════════════════════════════════════════════════════════╝
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <stdio.h>
#include <string.h>

/* ── Pins ── */
#define CLK_PIN 16
#define DIO_PIN 17

#define STACK_SIZE 1024
#define PRIORITY   7

static const struct device *gpio_dev;

/* ── Define the Message Queue ── */
/* 
 * We will pass an integer (temperature) between threads.
 * Size of message = sizeof(int)
 * Max messages in queue = 10
 */
K_MSGQ_DEFINE(temp_msgq, sizeof(int), 10, 4);

/* ── Character map for TM1637 ── */
static uint8_t get_char_code(char c) {
	switch (c) {
		case '0': return 0x3F; case '1': return 0x06;
		case '2': return 0x5B; case '3': return 0x4F;
		case '4': return 0x66; case '5': return 0x6D;
		case '6': return 0x7D; case '7': return 0x07;
		case '8': return 0x7F; case '9': return 0x6F;
		case 'C': return 0x39; case ' ': return 0x00;
		default:  return 0x00;
	}
}

/* ── TM1637 Bit-Bang Functions ── */
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

static void display_temp(int temp) {
	char str[5];
	snprintf(str, sizeof(str), "%2d C", temp); /* Format like "25 C" */

	start();
	write_byte(0x40);
	stop();

	start();
	write_byte(0xC0);
	for (int i = 0; i < 4; i++) { write_byte(get_char_code(str[i])); }
	stop();

	start();
	write_byte(0x8A);
	stop();
}

/* ─────────────────────────────────────────────────────────────
 * Thread 1: Sensor Task (Producer)
 * Generates a temperature value and puts it in the message queue.
 * ───────────────────────────────────────────────────────────── */
void sensor_thread(void *a, void *b, void *c)
{
	int mock_temperature = 20;
	while (1) {
		/* Send the temperature into the queue */
		k_msgq_put(&temp_msgq, &mock_temperature, K_NO_WAIT);
		
		/* Increment temp to simulate changes */
		mock_temperature++;
		if (mock_temperature > 35) { mock_temperature = 20; }
		
		k_sleep(K_MSEC(2000)); /* Read sensor every 2 seconds */
	}
}

/* ─────────────────────────────────────────────────────────────
 * Thread 2: Display Task (Consumer)
 * Waits for a message in the queue, then displays it.
 * ───────────────────────────────────────────────────────────── */
void display_thread(void *a, void *b, void *c)
{
	int received_temp;
	while (1) {
		/* Block until a message arrives in the queue */
		if (k_msgq_get(&temp_msgq, &received_temp, K_FOREVER) == 0) {
			display_temp(received_temp);
		}
	}
}

/* ── Spawn the threads ── */
K_THREAD_DEFINE(thread_sensor, STACK_SIZE, sensor_thread, NULL, NULL, NULL, PRIORITY, 0, 100);
K_THREAD_DEFINE(thread_display, STACK_SIZE, display_thread, NULL, NULL, NULL, PRIORITY, 0, 100);

/* ── Main Setup ── */
int main(void)
{
	gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));
	if (!device_is_ready(gpio_dev)) {
		return -1;
	}

	gpio_pin_configure(gpio_dev, CLK_PIN, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure(gpio_dev, DIO_PIN, GPIO_OUTPUT | GPIO_OPEN_DRAIN);
	gpio_pin_set(gpio_dev, CLK_PIN, 1);
	gpio_pin_set(gpio_dev, DIO_PIN, 1);

	return 0;
}
