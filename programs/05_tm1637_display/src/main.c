/*
 * ╔══════════════════════════════════════════════════════════════╗
 * ║     TM1637 Display & Non-Blocking Timers in Zephyr           ║
 * ║   Converted from MicroPython for Raspberry Pi Pico 2         ║
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
 * ║   Heartbeat    LED  → GP14 (+ 330Ω resistor) to GND          ║
 * ╚══════════════════════════════════════════════════════════════╝
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <stdio.h>
#include <string.h>

/* ── Pins ── */
#define CLK_PIN 16
#define DIO_PIN 17
#define LED_PIN 14

static const struct device *gpio_dev;

/* ── Character map for TM1637 ── */
static uint8_t get_char_code(char c) {
	switch (c) {
		case '0': return 0x3F; case '1': return 0x06;
		case '2': return 0x5B; case '3': return 0x4F;
		case '4': return 0x66; case '5': return 0x6D;
		case '6': return 0x7D; case '7': return 0x07;
		case '8': return 0x7F; case '9': return 0x6F;
		case 'A': return 0x77; case 'B': return 0x7C;
		case 'C': return 0x39; case 'D': return 0x5E;
		case 'E': return 0x79; case 'F': return 0x71;
		case 'G': return 0x3D; case 'H': return 0x76;
		case 'I': return 0x30; case 'L': return 0x38;
		case 'N': return 0x54; case 'O': return 0x3F;
		case 'P': return 0x73; case 'R': return 0x50;
		case 'S': return 0x6D; case 'T': return 0x78;
		case 'U': return 0x3E; case 'Y': return 0x6E;
		case '-': return 0x40;
		default:  return 0x00; /* Space */
	}
}

/* ── TM1637 Bit-Bang Functions ── */
/* Using Zephyr's k_busy_wait(20) for 20 microsecond delays */

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
	/* Ensure DIO is an output */
	gpio_pin_configure(gpio_dev, DIO_PIN, GPIO_OUTPUT | GPIO_OPEN_DRAIN);
	
	for (int i = 0; i < 8; i++) {
		gpio_pin_set(gpio_dev, CLK_PIN, 0);
		gpio_pin_set(gpio_dev, DIO_PIN, b & 1);
		k_busy_wait(20);
		gpio_pin_set(gpio_dev, CLK_PIN, 1);
		k_busy_wait(20);
		b >>= 1;
	}
	
	/* Wait for ACK */
	gpio_pin_set(gpio_dev, CLK_PIN, 0);
	/* Set DIO to input so the TM1637 can pull it low */
	gpio_pin_configure(gpio_dev, DIO_PIN, GPIO_INPUT | GPIO_PULL_UP);
	gpio_pin_set(gpio_dev, CLK_PIN, 1);
	k_busy_wait(20);
	
	/* Clock low again, restore DIO to output */
	gpio_pin_set(gpio_dev, CLK_PIN, 0);
	gpio_pin_configure(gpio_dev, DIO_PIN, GPIO_OUTPUT | GPIO_OPEN_DRAIN);
}

static void display_text(const char *text) {
	char padded[5] = "    ";
	int len = strlen(text);
	
	if (len > 4) len = 4;
	for (int i = 0; i < len; i++) {
		padded[i] = text[i];
	}

	start();
	write_byte(0x40); /* Data command: Write data to display register, auto increment address */
	stop();

	start();
	write_byte(0xC0); /* Address command: Set starting address to 00H */
	for (int i = 0; i < 4; i++) {
		write_byte(get_char_code(padded[i]));
	}
	stop();

	start();
	write_byte(0x8A); /* Display control: Display ON, Pulse width 10/16 */
	stop();
}

/* ── Main Task Loop ── */
int main(void) {
	gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));
	if (!device_is_ready(gpio_dev)) {
		return -1;
	}

	/* Initialize pins */
	gpio_pin_configure(gpio_dev, LED_PIN, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure(gpio_dev, CLK_PIN, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure(gpio_dev, DIO_PIN, GPIO_OUTPUT | GPIO_OPEN_DRAIN);
	
	/* Ensure line is high idle */
	gpio_pin_set(gpio_dev, CLK_PIN, 1);
	gpio_pin_set(gpio_dev, DIO_PIN, 1);

	/* Timers and state */
	uint32_t HEARTBEAT_INTERVAL = 1000;
	uint32_t FLASH_DURATION = 100;
	uint32_t DISPLAY_INTERVAL = 500;

	uint32_t last_heartbeat_time = k_uptime_get_32();
	uint32_t flash_start_time = 0;
	bool is_flashing = false;

	uint32_t last_display_time = k_uptime_get_32();
	int counter_value = 0;

	while (1) {
		uint32_t current_time = k_uptime_get_32();

		/* Task 1: Heartbeat Pulse Trigger (Every 1 second) */
		if ((current_time - last_heartbeat_time) >= HEARTBEAT_INTERVAL) {
			gpio_pin_set(gpio_dev, LED_PIN, 1); /* Turn LED ON */
			flash_start_time = current_time;
			is_flashing = true;
			last_heartbeat_time = current_time;
		}

		/* Turn the heartbeat LED OFF after the flash duration passes */
		if (is_flashing && ((current_time - flash_start_time) >= FLASH_DURATION)) {
			gpio_pin_set(gpio_dev, LED_PIN, 0); /* Turn LED OFF */
			is_flashing = false;
		}

		/* Task 2: Display Update Task (Increments number and refreshes every 500 ms) */
		if ((current_time - last_display_time) >= DISPLAY_INTERVAL) {
			char text[16];
			snprintf(text, sizeof(text), "%d", counter_value);
			display_text(text);

			counter_value++;
			if (counter_value > 9999) {
				counter_value = 0;
			}

			last_display_time = current_time;
		}

		/* Small sleep to keep CPU cool (yields to other threads) */
		k_sleep(K_MSEC(10));
	}

	return 0;
}
