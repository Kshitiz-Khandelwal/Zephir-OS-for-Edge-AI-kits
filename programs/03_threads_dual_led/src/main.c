/*
 * ╔══════════════════════════════════════════════════════════════╗
 * ║      Dual LED Blink — Zephyr RTOS Multi-Threading           ║
 * ║   Converted from MicroPython for Raspberry Pi Pico 2        ║
 * ╠══════════════════════════════════════════════════════════════╣
 * ║  WIRING                                                      ║
 * ║                                                              ║
 * ║   Pico 2 Pin  →  Component                                   ║
 * ║   ──────────     ─────────────────────────────               ║
 * ║   GP14       →  330Ω resistor → LED1 (+) → GND              ║
 * ║   GP15       →  330Ω resistor → LED2 (+) → GND              ║
 * ║                                                              ║
 * ║  LED1 blinks every 500 ms  (Thread 1 — main thread)         ║
 * ║  LED2 blinks every 200 ms  (Thread 2 — spawned thread)      ║
 * ╚══════════════════════════════════════════════════════════════╝
 *
 *  MicroPython → Zephyr RTOS comparison:
 *  ─────────────────────────────────────────────────────────────
 *  _thread.start_new_thread(fn, ())  →  K_THREAD_DEFINE(...)
 *  time.sleep(0.5)                   →  k_sleep(K_MSEC(500))
 *  led1.toggle()                     →  gpio_pin_toggle(...)
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/* ── Pin assignments — same as MicroPython Pin(14) / Pin(15) ── */
#define LED1_PIN  14   /* GP14 — blinks every 500 ms */
#define LED2_PIN  15   /* GP15 — blinks every 200 ms */

/* ── Thread stack sizes ────────────────────────────────────── */
#define STACK_SIZE  512
#define PRIORITY    5

/* ── Shared GPIO device ────────────────────────────────────── */
static const struct device *gpio_dev;

/* ─────────────────────────────────────────────────────────────
 * Thread 1 — blink_led1()
 * Equivalent to: MicroPython blink_led1() running in main
 * Toggles GP14 every 500 ms
 * ───────────────────────────────────────────────────────────── */
void blink_led1(void *a, void *b, void *c)
{
	while (1) {
		gpio_pin_toggle(gpio_dev, LED1_PIN);
		k_sleep(K_MSEC(500));   /* time.sleep(0.5) */
	}
}

/* ─────────────────────────────────────────────────────────────
 * Thread 2 — blink_led2()
 * Equivalent to: _thread.start_new_thread(blink_led2, ())
 * Toggles GP15 every 200 ms — runs concurrently with Thread 1
 * ───────────────────────────────────────────────────────────── */
void blink_led2(void *a, void *b, void *c)
{
	while (1) {
		gpio_pin_toggle(gpio_dev, LED2_PIN);
		k_sleep(K_MSEC(200));   /* time.sleep(0.2) */
	}
}

/* ── Define Thread 2 statically (Zephyr's equivalent of _thread) */
K_THREAD_DEFINE(
	thread2,           /* thread name */
	STACK_SIZE,        /* stack size in bytes */
	blink_led2,        /* entry function */
	NULL, NULL, NULL,  /* arguments (none) */
	PRIORITY,          /* priority */
	0,                 /* options */
	0                  /* delay before start (ms) */
);

/* ── Main = Thread 1 ───────────────────────────────────────── */
int main(void)
{
	/* Get GPIO0 controller */
	gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));
	if (!device_is_ready(gpio_dev)) {
		return -1;
	}

	/* Configure both LED pins as outputs, start LOW */
	gpio_pin_configure(gpio_dev, LED1_PIN, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure(gpio_dev, LED2_PIN, GPIO_OUTPUT_INACTIVE);

	/* Thread 2 already started by K_THREAD_DEFINE above */
	/* Run Thread 1 (LED1) here in main */
	blink_led1(NULL, NULL, NULL);

	return 0;
}
