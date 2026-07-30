/*
 * ╔══════════════════════════════════════════════════════════════╗
 * ║     Software Timers in Zephyr RTOS                           ║
 * ║                                                              ║
 * ╠══════════════════════════════════════════════════════════════╣
 * ║  WIRING                                                      ║
 * ║                                                              ║
 * ║   Component    Pin    Raspberry Pi Pico                      ║
 * ║   ─────────    ───    ─────────────────                      ║
 * ║   LED          OUT  → GP14 (+ 330Ω resistor) to GND          ║
 * ╚══════════════════════════════════════════════════════════════╝
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/* ── Pins ── */
#define LED_PIN 14

static const struct device *gpio_dev;

/* ── Timer Expiry Callback ── */
/* This function is automatically called by the Zephyr RTOS 
 * whenever the software timer expires. It runs in an interrupt-like context.
 */
void my_timer_expiry_function(struct k_timer *timer_id)
{
	/* Toggle the LED */
	gpio_pin_toggle(gpio_dev, LED_PIN);
}

/* ── Define the Software Timer ── */
/* Name: my_timer
 * Expiry function: my_timer_expiry_function
 * Stop function: NULL (we don't need one)
 */
K_TIMER_DEFINE(my_timer, my_timer_expiry_function, NULL);

/* ── Main Setup ── */
int main(void)
{
	gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));
	if (!device_is_ready(gpio_dev)) {
		return -1;
	}

	gpio_pin_configure(gpio_dev, LED_PIN, GPIO_OUTPUT_INACTIVE);

	/* Start the timer. 
	 * First duration: K_SECONDS(5) -> Wait 5s before first toggle
	 * Period: K_SECONDS(5) -> Toggle every 5 seconds thereafter
	 */
	k_timer_start(&my_timer, K_SECONDS(5), K_SECONDS(5));

	/* Notice there is NO while(1) loop or k_sleep() delay loop!
	 * The main thread simply exits. 
	 * The RTOS background kernel timer handles everything automatically.
	 */
	return 0;
}
