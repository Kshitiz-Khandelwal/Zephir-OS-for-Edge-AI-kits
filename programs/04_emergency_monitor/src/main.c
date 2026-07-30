/*
 * ╔══════════════════════════════════════════════════════════════╗
 * ║      Emergency Monitor & Non-Blocking Timers in Zephyr       ║
 * ║   Converted from MicroPython for Raspberry Pi Pico 2         ║
 * ╠══════════════════════════════════════════════════════════════╣
 * ║  WIRING                                                      ║
 * ║                                                              ║
 * ║   Pico 2 Pin  →  Component                                   ║
 * ║   ──────────     ─────────────────────────────               ║
 * ║   GP14       →  330Ω resistor → LED_FAST (+) → GND           ║
 * ║   GP15       →  330Ω resistor → LED_SLOW (+) → GND           ║
 * ║   GP13       →  330Ω resistor → LED_EMERGENCY (+) → GND      ║
 * ║   GP16       →  Touch Sensor Output (or Button)              ║
 * ║                                                              ║
 * ║  LED_FAST (GP14) blinks every 200 ms via non-blocking timer  ║
 * ║  LED_SLOW (GP15) blinks every 1000 ms via non-blocking timer ║
 * ║  Thread 2 monitors GP16. If HIGH, flashes GP13 quickly.      ║
 * ╚══════════════════════════════════════════════════════════════╝
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/* ── Pin assignments ── */
#define LED_FAST_PIN       14
#define LED_SLOW_PIN       15
#define LED_EMERGENCY_PIN  13
#define TOUCH_SENSOR_PIN   16

/* ── Thread stack sizes ────────────────────────────────────── */
#define STACK_SIZE  1024
#define PRIORITY    5

/* ── Shared GPIO device ────────────────────────────────────── */
static const struct device *gpio_dev;

/* ─────────────────────────────────────────────────────────────
 * Thread 2 — Emergency Monitor
 * Checks the touch sensor pin continuously. If triggered,
 * flashes the emergency LED 5 times rapidly.
 * ───────────────────────────────────────────────────────────── */
void emergency_monitor(void *a, void *b, void *c)
{
	while (1) {
		/* Read the sensor value */
		int val = gpio_pin_get(gpio_dev, TOUCH_SENSOR_PIN);
		
		if (val == 1) {
			/* Emergency action: flash fast immediately */
			for (int i = 0; i < 5; i++) {
				gpio_pin_toggle(gpio_dev, LED_EMERGENCY_PIN);
				k_sleep(K_MSEC(50));
				gpio_pin_toggle(gpio_dev, LED_EMERGENCY_PIN);
				k_sleep(K_MSEC(50));
			}
			/* Ensure it's off afterwards */
			gpio_pin_set(gpio_dev, LED_EMERGENCY_PIN, 0);
		}
		
		/* High-frequency check: time.sleep(0.01) -> 10ms */
		k_sleep(K_MSEC(10));
	}
}

/* ── Define Thread 2 statically ── */
K_THREAD_DEFINE(
	monitor_thread,    /* thread name */
	STACK_SIZE,        /* stack size in bytes */
	emergency_monitor, /* entry function */
	NULL, NULL, NULL,  /* arguments */
	PRIORITY,          /* priority */
	0,                 /* options */
	0                  /* delay before start (ms) */
);

/* ─────────────────────────────────────────────────────────────
 * Thread 1 — Main loop with non-blocking timers
 * ───────────────────────────────────────────────────────────── */
int main(void)
{
	/* Get GPIO0 controller */
	gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));
	if (!device_is_ready(gpio_dev)) {
		return -1;
	}

	/* Configure pins */
	gpio_pin_configure(gpio_dev, LED_FAST_PIN, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure(gpio_dev, LED_SLOW_PIN, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure(gpio_dev, LED_EMERGENCY_PIN, GPIO_OUTPUT_INACTIVE);
	
	/* Configure touch sensor as input with pull-down */
	gpio_pin_configure(gpio_dev, TOUCH_SENSOR_PIN, GPIO_INPUT | GPIO_PULL_DOWN);

	/* Timers setup */
	uint32_t fast_interval = 200;  /* 0.2 seconds */
	uint32_t slow_interval = 1000; /* 1.0 seconds */

	uint32_t last_fast_time = k_uptime_get_32();
	uint32_t last_slow_time = k_uptime_get_32();

	while (1) {
		uint32_t current_time = k_uptime_get_32();

		/* Check if it's time to toggle the fast LED */
		if ((current_time - last_fast_time) >= fast_interval) {
			gpio_pin_toggle(gpio_dev, LED_FAST_PIN);
			last_fast_time = current_time;
		}

		/* Check if it's time to toggle the slow LED */
		if ((current_time - last_slow_time) >= slow_interval) {
			gpio_pin_toggle(gpio_dev, LED_SLOW_PIN);
			last_slow_time = current_time;
		}

		/* Tiny sleep to keep the loop smooth and yield to other threads */
		k_sleep(K_MSEC(10));
	}

	return 0;
}
