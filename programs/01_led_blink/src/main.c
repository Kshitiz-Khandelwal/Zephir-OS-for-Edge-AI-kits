/*
 * ╔══════════════════════════════════════════════════════╗
 * ║        3-LED Blink Test — Zephyr RTOS               ║
 * ║   Converted from MicroPython for Raspberry Pi Pico 2 ║
 * ╠══════════════════════════════════════════════════════╣
 * ║  WIRING                                              ║
 * ║                                                      ║
 * ║   Pico 2 Pin  →  Component                           ║
 * ║   ──────────     ─────────────────────────           ║
 * ║   GP0        →  330Ω resistor → RED LED  → GND      ║
 * ║   GP1        →  330Ω resistor → GREEN LED → GND     ║
 * ║   GP2        →  330Ω resistor → BLUE LED → GND      ║
 * ║                                                      ║
 * ║  NOTE: Always use a resistor with LEDs!              ║
 * ║        Longer leg of LED (+) faces the resistor      ║
 * ╚══════════════════════════════════════════════════════╝
 *
 *  MicroPython original:          Zephyr equivalent:
 *  ─────────────────────          ──────────────────
 *  Pin(0, Pin.OUT)                gpio_pin_configure(..., GPIO_OUTPUT)
 *  red.on()                       gpio_pin_set(..., RED_PIN, 1)
 *  red.off()                      gpio_pin_set(..., RED_PIN, 0)
 *  sleep(1)                       k_sleep(K_SECONDS(1))
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/* Pin numbers — same as MicroPython Pin(0), Pin(1), Pin(2) */
#define RED_PIN    0   /* GP0 */
#define GREEN_PIN  1   /* GP1 */
#define BLUE_PIN   2   /* GP2 */

int main(void)
{
	/* Get GPIO controller (all Pico 2 pins are on gpio0) */
	const struct device *gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));

	if (!device_is_ready(gpio_dev)) {
		return -1;
	}

	/* Configure all 3 pins as outputs, start LOW (off) */
	gpio_pin_configure(gpio_dev, RED_PIN,   GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure(gpio_dev, GREEN_PIN, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure(gpio_dev, BLUE_PIN,  GPIO_OUTPUT_INACTIVE);

	while (1) {
		/* Turn ON all LEDs */
		gpio_pin_set(gpio_dev, RED_PIN,   1);
		gpio_pin_set(gpio_dev, GREEN_PIN, 1);
		gpio_pin_set(gpio_dev, BLUE_PIN,  1);
		k_sleep(K_SECONDS(1));

		/* Turn OFF all LEDs */
		gpio_pin_set(gpio_dev, RED_PIN,   0);
		gpio_pin_set(gpio_dev, GREEN_PIN, 0);
		gpio_pin_set(gpio_dev, BLUE_PIN,  0);
		k_sleep(K_SECONDS(1));
	}

	return 0;
}
