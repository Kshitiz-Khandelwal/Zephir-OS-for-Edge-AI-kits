/*
 * ╔══════════════════════════════════════════════════════════════╗
 * ║     Thread Synchronization via Semaphores in Zephyr RTOS     ║
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
 * ║                                                              ║
 * ║   Joystick SW  SW   → GP22 (Internal Pull-Up)                ║
 * ║   Joystick GND GND  → GND                                    ║
 * ╚══════════════════════════════════════════════════════════════╝
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <stdio.h>
#include <string.h>

/* ── Pins ── */
#define CLK_PIN 16
#define DIO_PIN 17
#define SW_PIN  22

#define STACK_SIZE 1024
#define PRIORITY   7

static const struct device *gpio_dev;

/* ── Define the Semaphore ── */
/* Initial count = 0, Limit = 1 */
K_SEM_DEFINE(button_sem, 0, 1);

/* ── Character map for TM1637 ── */
static uint8_t get_char_code(char c) {
	switch (c) {
		case 'E': return 0x79; case 'K': return 0x76; /* Approximate K with H */
		case 'L': return 0x38; case 'N': return 0x54;
		case 'O': return 0x3F; case 'P': return 0x73;
		case 'C': return 0x39;
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

static void display_text(const char *text) {
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
}

/* ─────────────────────────────────────────────────────────────
 * Thread 1: Input Task
 * Polls the joystick switch. If pressed (LOW), gives semaphore.
 * ───────────────────────────────────────────────────────────── */
void input_thread(void *a, void *b, void *c)
{
	while (1) {
		int val = gpio_pin_get(gpio_dev, SW_PIN);
		if (val == 0) { /* Button is pressed (active low) */
			k_sem_give(&button_sem);
		}
		k_sleep(K_MSEC(50));
	}
}

/* ─────────────────────────────────────────────────────────────
 * Thread 2: Display Task
 * Blocks waiting for the semaphore.
 * ───────────────────────────────────────────────────────────── */
void display_thread(void *a, void *b, void *c)
{
	/* Let's use a 100ms timeout on the semaphore.
	 * If the button is held, input_thread gives the semaphore every 50ms,
	 * so take will succeed.
	 * If released, it will time out.
	 */
	while (1) {
		if (k_sem_take(&button_sem, K_MSEC(100)) == 0) {
			/* Semaphore successfully taken -> Button is held */
			display_text("OPEN");
		} else {
			/* Timeout -> Button is NOT held */
			display_text("LOC "); /* Note: Using LOC since 'K' is rendered as 'H' on 7-seg */
		}
	}
}

/* ── Spawn the threads ── */
K_THREAD_DEFINE(thread_input, STACK_SIZE, input_thread, NULL, NULL, NULL, PRIORITY, 0, 100);
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
	
	/* Configure Joystick SW as Input with Pull-Up */
	gpio_pin_configure(gpio_dev, SW_PIN, GPIO_INPUT | GPIO_PULL_UP);

	return 0;
}
