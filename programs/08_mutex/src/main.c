/*
 * ╔══════════════════════════════════════════════════════════════╗
 * ║     Resource Protection via Mutexes in Zephyr RTOS           ║
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

/* ── Define the Mutex ── */
/* This mutex ensures only one thread can talk to the display at a time */
K_MUTEX_DEFINE(display_mutex);

/* ── Character map for TM1637 ── */
static uint8_t get_char_code(char c) {
	switch (c) {
		case 'A': return 0x77; case 'D': return 0x5E;
		case 'G': return 0x3D; case 'I': return 0x30;
		case 'N': return 0x54; case 'P': return 0x73;
		case 'T': return 0x78;
		default:  return 0x00; /* Space */
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

/* ── Thread-Safe Display Function ── */
static void safe_display_text(const char *text) {
	/* Lock the Mutex! If another thread is currently holding it,
	 * this thread will block (sleep) right here until it's unlocked. */
	k_mutex_lock(&display_mutex, K_FOREVER);

	char padded[5] = "    ";
	int len = strlen(text);
	if (len > 4) len = 4;
	for (int i = 0; i < len; i++) { padded[i] = text[i]; }

	start();
	write_byte(0x40);
	stop();

	start();
	write_byte(0xC0);
	for (int i = 0; i < 4; i++) { write_byte(get_char_code(padded[i])); }
	stop();

	start();
	write_byte(0x8A);
	stop();

	/* We are done communicating with the display. Unlock the Mutex!
	 * Any other threads waiting to write to the display will now wake up. */
	k_mutex_unlock(&display_mutex);
}

/* ─────────────────────────────────────────────────────────────
 * Thread 1: Sensor Task
 * Displays "DATA" every 1.5 seconds.
 * ───────────────────────────────────────────────────────────── */
void sensor_thread(void *a, void *b, void *c)
{
	while (1) {
		safe_display_text("DATA");
		k_sleep(K_MSEC(1500));
	}
}

/* ─────────────────────────────────────────────────────────────
 * Thread 2: Main Task
 * Displays "PING" every 1.0 seconds.
 * ───────────────────────────────────────────────────────────── */
void main_thread(void *a, void *b, void *c)
{
	while (1) {
		safe_display_text("PING");
		k_sleep(K_MSEC(1000));
	}
}

/* ── Spawn the threads ── */
K_THREAD_DEFINE(thread_sensor, STACK_SIZE, sensor_thread, NULL, NULL, NULL, PRIORITY, 0, 100);
K_THREAD_DEFINE(thread_main, STACK_SIZE, main_thread, NULL, NULL, NULL, PRIORITY, 0, 100);

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

	/* Main thread just exits. The spawned threads do all the work. */
	return 0;
}
