/*
 * ╔══════════════════════════════════════════════════════════════╗
 * ║     Thread Priorities & Preemption in Zephyr RTOS            ║
 * ║                                                              ║
 * ╠══════════════════════════════════════════════════════════════╣
 * ║  WIRING                                                      ║
 * ║                                                              ║
 * ║   Component    Pin    Raspberry Pi Pico                      ║
 * ║   ─────────    ───    ─────────────────                      ║
 * ║   LED 1 (+)    AMB  → GP14 (+ 330Ω resistor) to GND          ║
 * ║   LED 2 (+)    RED  → GP15 (+ 330Ω resistor) to GND          ║
 * ║   LED Emrg(+)  GRN  → GP13 (+ 330Ω resistor) to GND          ║
 * ║                                                              ║
 * ║   Button/Sen   OUT  → GP16 (with VCC to 3.3V, GND to GND)    ║
 * ╚══════════════════════════════════════════════════════════════╝
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/* ── Pins ── */
#define LED1_PIN       14
#define LED2_PIN       15
#define LED_EMERG_PIN  13
#define BUTTON_PIN     16

/* ── Thread Priorities ── */
/* In Zephyr, lower number = higher priority */
#define PRIORITY_HIGH    2   /* Emergency thread */
#define PRIORITY_NORMAL  7   /* Standard blinking threads */

#define STACK_SIZE 1024

static const struct device *gpio_dev;

/* ─────────────────────────────────────────────────────────────
 * Thread 1: Normal Priority (Blinks LED 1)
 * ───────────────────────────────────────────────────────────── */
void normal_thread_1(void *a, void *b, void *c)
{
	while (1) {
		gpio_pin_toggle(gpio_dev, LED1_PIN);
		k_sleep(K_MSEC(500));
	}
}

/* ─────────────────────────────────────────────────────────────
 * Thread 2: Normal Priority (Blinks LED 2)
 * ───────────────────────────────────────────────────────────── */
void normal_thread_2(void *a, void *b, void *c)
{
	while (1) {
		gpio_pin_toggle(gpio_dev, LED2_PIN);
		k_sleep(K_MSEC(1000));
	}
}

/* ─────────────────────────────────────────────────────────────
 * Thread 3: High Priority (Emergency Monitor)
 * ───────────────────────────────────────────────────────────── */
void emergency_thread(void *a, void *b, void *c)
{
	while (1) {
		int val = gpio_pin_get(gpio_dev, BUTTON_PIN);
		
		if (val == 1) {
			/* Button pressed! Because this thread has Priority 2,
			 * it instantly preempts (pauses) the Priority 7 threads. 
			 * They will NOT execute while this thread is actively running. */
			for (int i = 0; i < 10; i++) {
				gpio_pin_toggle(gpio_dev, LED_EMERG_PIN);
				k_sleep(K_MSEC(50)); 
			}
			gpio_pin_set(gpio_dev, LED_EMERG_PIN, 0); /* Ensure off */
		}
		
		/* Sleep to allow lower priority threads to run */
		k_sleep(K_MSEC(10));
	}
}

/* ── Spawn the threads ── */
K_THREAD_DEFINE(thread_n1, STACK_SIZE, normal_thread_1, NULL, NULL, NULL, PRIORITY_NORMAL, 0, 100);
K_THREAD_DEFINE(thread_n2, STACK_SIZE, normal_thread_2, NULL, NULL, NULL, PRIORITY_NORMAL, 0, 100);
K_THREAD_DEFINE(thread_em, STACK_SIZE, emergency_thread, NULL, NULL, NULL, PRIORITY_HIGH, 0, 100);

/* ── Main Setup ── */
int main(void)
{
	gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));
	if (!device_is_ready(gpio_dev)) {
		return -1;
	}

	gpio_pin_configure(gpio_dev, LED1_PIN, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure(gpio_dev, LED2_PIN, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure(gpio_dev, LED_EMERG_PIN, GPIO_OUTPUT_INACTIVE);
	
	/* Button is input with pull-down */
	gpio_pin_configure(gpio_dev, BUTTON_PIN, GPIO_INPUT | GPIO_PULL_DOWN);

	/* Main thread just exits. The spawned threads do all the work. */
	return 0;
}
