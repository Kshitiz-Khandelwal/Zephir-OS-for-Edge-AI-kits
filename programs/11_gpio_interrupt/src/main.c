/*
 * ╔══════════════════════════════════════════════════════════════╗
 * ║     Hardware Interrupts in Zephyr RTOS (GPIO ISR)            ║
 * ║                                                              ║
 * ╠══════════════════════════════════════════════════════════════╣
 * ║  WIRING                                                      ║
 * ║                                                              ║
 * ║   Component    Pin    Raspberry Pi Pico                      ║
 * ║   ─────────    ───    ─────────────────                      ║
 * ║   Button       OUT  → GP2 (Internal Pull-Up)                 ║
 * ║   Button       GND  → GND                                    ║
 * ║                                                              ║
 * ║   LED          OUT  → GP14 (+ 330Ω resistor) to GND          ║
 * ╚══════════════════════════════════════════════════════════════╝
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <stdio.h>

/* ── Pins ── */
#define BUTTON_PIN 2
#define LED_PIN    14

static const struct device *gpio_dev;
static struct gpio_callback button_cb_data;

/* ── Hardware Interrupt Service Routine (ISR) ── */
/* 
 * This function is called instantly by the CPU hardware the millisecond 
 * the button is pressed. It interrupts whatever the background task was doing.
 */
void button_pressed_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	/* Instantly turn on the LED and print to console */
	gpio_pin_set(gpio_dev, LED_PIN, 1);
	printf("\n*** INTERRUPT: Button Touched! ***\n");
	
	/* We shouldn't sleep or do heavy work inside an ISR, 
	 * so we just do something quick and get out. */
}

/* ── Main Setup & Background Task ── */
int main(void)
{
	gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));
	if (!device_is_ready(gpio_dev)) {
		return -1;
	}

	/* Configure LED */
	gpio_pin_configure(gpio_dev, LED_PIN, GPIO_OUTPUT_INACTIVE);

	/* Configure Button with Internal Pull-Up */
	gpio_pin_configure(gpio_dev, BUTTON_PIN, GPIO_INPUT | GPIO_PULL_UP);

	/* Configure Hardware Interrupt to trigger on the falling edge (button press) */
	gpio_pin_interrupt_configure(gpio_dev, BUTTON_PIN, GPIO_INT_EDGE_TO_ACTIVE);

	/* Register our callback function to this pin */
	gpio_init_callback(&button_cb_data, button_pressed_isr, BIT(BUTTON_PIN));
	gpio_add_callback(gpio_dev, &button_cb_data);

	/* --- BACKGROUND TASK --- */
	/* This represents a normal, slow, long-running task. 
	 * Even while it is sleeping, the hardware interrupt will still fire instantly! */
	while (1) {
		printf("=");
		
		/* Turn LED off after a bit so we can see the flash again next interrupt */
		k_sleep(K_MSEC(100));
		gpio_pin_set(gpio_dev, LED_PIN, 0);

		/* Long sleep. If we used simple polling, button presses during 
		 * this 800ms sleep would be completely ignored! 
		 * But because we use Hardware Interrupts, it works perfectly. */
		k_sleep(K_MSEC(800));
	}

	return 0;
}
