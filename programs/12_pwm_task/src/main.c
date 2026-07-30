/*
 * ╔══════════════════════════════════════════════════════════════╗
 * ║     Concurrent Tasks: Software PWM and Status Printer        ║
 * ║                                                              ║
 * ╠══════════════════════════════════════════════════════════════╣
 * ║  WIRING                                                      ║
 * ║                                                              ║
 * ║   Component    Pin    Raspberry Pi Pico                      ║
 * ║   ─────────    ───    ─────────────────                      ║
 * ║   Red LED      OUT  → GP15 (+ 330Ω resistor) to GND          ║
 * ║   Green LED    OUT  → GP14 (+ 330Ω resistor) to GND          ║
 * ║   Blue LED     OUT  → GP13 (+ 330Ω resistor) to GND          ║
 * ╚══════════════════════════════════════════════════════════════╝
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <stdio.h>

/* ── Pins ── */
#define RED_PIN   15
#define GREEN_PIN 14
#define BLUE_PIN  13

#define STACK_SIZE 1024
#define PRIORITY   7

static const struct device *gpio_dev;

/* ─────────────────────────────────────────────────────────────
 * Thread 1: System Status
 * Prints a status message every 1 second.
 * ───────────────────────────────────────────────────────────── */
void status_thread(void *a, void *b, void *c)
{
	while (1) {
		printf("System Status : Running\n");
		k_sleep(K_MSEC(1000));
	}
}

/* ─────────────────────────────────────────────────────────────
 * Thread 2: Main Task (Software PWM)
 * Rapidly toggles the LEDs to simulate PWM fading.
 * Because Zephyr is a preemptive RTOS, this rapid toggling 
 * does not stop the Status Thread from printing!
 * ───────────────────────────────────────────────────────────── */
int main(void)
{
	gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));
	if (!device_is_ready(gpio_dev)) {
		return -1;
	}

	gpio_pin_configure(gpio_dev, RED_PIN, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure(gpio_dev, GREEN_PIN, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure(gpio_dev, BLUE_PIN, GPIO_OUTPUT_INACTIVE);

	/* 
	 * Software PWM Variables 
	 * We use a 2000 microsecond (2ms) period which equals 500Hz.
	 */
	int duty = 0;
	int dir = 1;

	while (1) {
		/* Fading logic */
		duty += (dir * 20);
		if (duty >= 2000) {
			dir = -1;
			duty = 2000;
		} else if (duty <= 0) {
			dir = 1;
			duty = 0;
		}

		/* 
		 * Software PWM Generation
		 * Turn LEDs ON for 'duty' microseconds
		 */
		if (duty > 0) {
			gpio_pin_set(gpio_dev, RED_PIN, 1);
			/* We make Green the inverse of Red, and Blue half as bright */
			if ((2000 - duty) > 0) gpio_pin_set(gpio_dev, GREEN_PIN, 1); 
			if ((duty / 2) > 0)    gpio_pin_set(gpio_dev, BLUE_PIN, 1);
			
			k_busy_wait(duty);
		}

		/* 
		 * Turn LEDs OFF for the remainder of the 2ms period
		 */
		int off_time = 2000 - duty;
		if (off_time > 0) {
			gpio_pin_set(gpio_dev, RED_PIN, 0);
			gpio_pin_set(gpio_dev, GREEN_PIN, 0);
			gpio_pin_set(gpio_dev, BLUE_PIN, 0);
			
			k_busy_wait(off_time);
		}
		
		/* Yield slightly so the Status Thread can wake up exactly on time */
		k_yield();
	}

	return 0;
}

/* ── Spawn the Status Thread ── */
K_THREAD_DEFINE(thread_status, STACK_SIZE, status_thread, NULL, NULL, NULL, PRIORITY, 0, 100);
