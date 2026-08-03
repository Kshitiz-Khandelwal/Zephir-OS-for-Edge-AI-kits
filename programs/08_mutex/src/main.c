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
#include <zephyr/sys/printk.h>

/* ── Pins ── */
#define CLK_PIN 16
#define DIO_PIN 17

#define STACK_SIZE 1024
#define PRIORITY   7

static const struct device *gpio_dev;

/* ── Define the Mutex ── */
/* This mutex ensures only one thread can talk to the display at a time */
K_MUTEX_DEFINE(display_mutex);

/* ── 7-Segment Codes for 0-9 ── */
static const uint8_t digits[] = {
	0x3F, 0x06, 0x5B, 0x4F, 0x66,
	0x6D, 0x7D, 0x07, 0x7F, 0x6F
};

/* ── TM1637 Bit-Bang Functions ── */
static void start(void) {
	gpio_pin_configure(gpio_dev, DIO_PIN, GPIO_OUTPUT);
	gpio_pin_set(gpio_dev, DIO_PIN, 1);
	gpio_pin_set(gpio_dev, CLK_PIN, 1);
	k_busy_wait(5);
	gpio_pin_set(gpio_dev, DIO_PIN, 0);
	k_busy_wait(5);
	gpio_pin_set(gpio_dev, CLK_PIN, 0);
	k_busy_wait(5);
}

static void stop(void) {
	gpio_pin_configure(gpio_dev, DIO_PIN, GPIO_OUTPUT);
	gpio_pin_set(gpio_dev, CLK_PIN, 0);
	k_busy_wait(5);
	gpio_pin_set(gpio_dev, DIO_PIN, 0);
	k_busy_wait(5);
	gpio_pin_set(gpio_dev, CLK_PIN, 1);
	k_busy_wait(5);
	gpio_pin_set(gpio_dev, DIO_PIN, 1);
	k_busy_wait(5);
}

static void write_byte(uint8_t b) {
	gpio_pin_configure(gpio_dev, DIO_PIN, GPIO_OUTPUT);
	for (int i = 0; i < 8; i++) {
		gpio_pin_set(gpio_dev, CLK_PIN, 0);
		k_busy_wait(5);
		gpio_pin_set(gpio_dev, DIO_PIN, b & 1);
		k_busy_wait(5);
		gpio_pin_set(gpio_dev, CLK_PIN, 1);
		k_busy_wait(5);
		b >>= 1;
	}
	gpio_pin_set(gpio_dev, CLK_PIN, 0);
	k_busy_wait(5);
	gpio_pin_configure(gpio_dev, DIO_PIN, GPIO_INPUT | GPIO_PULL_UP);
	k_busy_wait(5);
	gpio_pin_set(gpio_dev, CLK_PIN, 1);
	k_busy_wait(5);
	gpio_pin_set(gpio_dev, CLK_PIN, 0);
	k_busy_wait(5);
	gpio_pin_configure(gpio_dev, DIO_PIN, GPIO_OUTPUT);
}

/* ── Thread-Safe Display Function ── */
static void safe_display_number(int number) {
	/* Lock the Mutex! If another thread is currently holding it,
	 * this thread will block (sleep) right here until it's unlocked. */
	k_mutex_lock(&display_mutex, K_FOREVER);

	number = number % 10000;
	int d1 = number / 1000;
	int d2 = (number / 100) % 10;
	int d3 = (number / 10) % 10;
	int d4 = number % 10;

	start();
	write_byte(0x40);
	stop();

	start();
	write_byte(0xC0);
	write_byte(digits[d1]);
	write_byte(digits[d2]);
	write_byte(digits[d3]);
	write_byte(digits[d4]);
	stop();

	start();
	write_byte(0x8F); /* 0x80 | 8 (display on) | 7 (brightness) */
	stop();

	/* We are done communicating with the display. Unlock the Mutex!
	 * Any other threads waiting to write to the display will now wake up. */
	k_mutex_unlock(&display_mutex);
}

/* ─────────────────────────────────────────────────────────────
 * Thread 1: Task 1
 * Displays and prints task1_counter every 1 second.
 * ───────────────────────────────────────────────────────────── */
static int task1_counter = 0;

void task1_thread(void *a, void *b, void *c)
{
	while (1) {
		safe_display_number(task1_counter);
		printk("Task 1 writing: %d\n", task1_counter);
		task1_counter++;
		k_sleep(K_MSEC(1000));
	}
}

/* ─────────────────────────────────────────────────────────────
 * Thread 2: Task 2
 * Displays and prints task2_counter every 2 seconds.
 * ───────────────────────────────────────────────────────────── */
static int task2_counter = 1000;

void task2_thread(void *a, void *b, void *c)
{
	while (1) {
		safe_display_number(task2_counter);
		printk("Task 2 writing: %d\n", task2_counter);
		task2_counter++;
		k_sleep(K_MSEC(2000));
	}
}

/* ── Spawn the threads ── */
K_THREAD_DEFINE(thread_1, STACK_SIZE, task1_thread, NULL, NULL, NULL, PRIORITY, 0, 100);
K_THREAD_DEFINE(thread_2, STACK_SIZE, task2_thread, NULL, NULL, NULL, PRIORITY, 0, 100);

/* ── Main Setup ── */
int main(void)
{
	gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));
	if (!device_is_ready(gpio_dev)) {
		return -1;
	}

	gpio_pin_configure(gpio_dev, CLK_PIN, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure(gpio_dev, DIO_PIN, GPIO_OUTPUT_INACTIVE);
	gpio_pin_set(gpio_dev, CLK_PIN, 1);
	gpio_pin_set(gpio_dev, DIO_PIN, 1);

	/* Main thread just exits. The spawned threads do all the work. */
	return 0;
}
