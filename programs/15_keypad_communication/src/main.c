/*
 * ╔══════════════════════════════════════════════════════════════╗
 * ║     Experiment 13: Task Communication (Keypad Producer)      ║
 * ║                                                              ║
 * ╠══════════════════════════════════════════════════════════════╣
 * ║  WIRING                                                      ║
 * ║                                                              ║
 * ║   Keypad Rows (Outputs)     Pico GPIO                        ║
 * ║   ─────────────────────     ─────────                        ║
 * ║   R1                        GP2                              ║
 * ║   R2                        GP3                              ║
 * ║   R3                        GP4                              ║
 * ║   R4                        GP5                              ║
 * ║                                                              ║
 * ║   Keypad Cols (Inputs)      Pico GPIO                        ║
 * ║   ─────────────────────     ─────────                        ║
 * ║   C1                        GP6 (Pull-up)                    ║
 * ║   C2                        GP7 (Pull-up)                    ║
 * ║   C3                        GP8 (Pull-up)                    ║
 * ║   C4                        GP9 (Pull-up)                    ║
 * ╚══════════════════════════════════════════════════════════════╝
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <stdio.h>

#define STACK_SIZE 1024
#define PRIORITY   7

static const struct device *gpio_dev;

/* ── Hardware Pins ── */
const int rows[4] = {2, 3, 4, 5};
const int cols[4] = {6, 7, 8, 9};

const char keys[4][4] = {
	{'1','2','3','A'},
	{'4','5','6','B'},
	{'7','8','9','C'},
	{'*','0','#','D'}
};

/* ── Shared Variables & Mutex ── */
char pressed_key = '\0';
K_MUTEX_DEFINE(keypad_mutex);

/* ─────────────────────────────────────────────────────────────
 * Task 1: Keypad Task (Producer)
 * Scans the matrix and updates the shared variable.
 * ───────────────────────────────────────────────────────────── */
void keypad_task(void *a, void *b, void *c)
{
	while (1) {
		for (int r = 0; r < 4; r++) {
			/* Make all rows HIGH */
			for (int i = 0; i < 4; i++) {
				gpio_pin_set(gpio_dev, rows[i], 1);
			}

			/* Drive one row LOW */
			gpio_pin_set(gpio_dev, rows[r], 0);

			/* Check columns */
			for (int c = 0; c < 4; c++) {
				if (gpio_pin_get(gpio_dev, cols[c]) == 0) {
					/* Key is pressed! */
					
					/* Protect the shared variable */
					k_mutex_lock(&keypad_mutex, K_FOREVER);
					pressed_key = keys[r][c];
					k_mutex_unlock(&keypad_mutex);

					/* Wait until key is released (Debounce) */
					while (gpio_pin_get(gpio_dev, cols[c]) == 0) {
						k_sleep(K_MSEC(10));
					}
					k_sleep(K_MSEC(200));
				}
			}
		}
		k_sleep(K_MSEC(10));
	}
}

/* ─────────────────────────────────────────────────────────────
 * Task 2: Display Task (Consumer)
 * Checks the shared variable and displays it if it changed.
 * ───────────────────────────────────────────────────────────── */
void display_task(void *a, void *b, void *c)
{
	char last_key = '\0';

	while (1) {
		/* Safely read the shared variable */
		k_mutex_lock(&keypad_mutex, K_FOREVER);
		char current_key = pressed_key;
		k_mutex_unlock(&keypad_mutex);

		if (current_key != '\0' && current_key != last_key) {
			printf("Key Received : %c\n", current_key);
			last_key = current_key;
		}

		k_sleep(K_MSEC(50));
	}
}

/* ── Spawn the Threads ── */
K_THREAD_DEFINE(thread_keypad, STACK_SIZE, keypad_task, NULL, NULL, NULL, PRIORITY, 0, 100);
K_THREAD_DEFINE(thread_display, STACK_SIZE, display_task, NULL, NULL, NULL, PRIORITY, 0, 100);

/* ── Main Setup ── */
int main(void)
{
	gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));
	if (!device_is_ready(gpio_dev)) {
		return -1;
	}

	/* Configure Rows as Output */
	for (int i = 0; i < 4; i++) {
		gpio_pin_configure(gpio_dev, rows[i], GPIO_OUTPUT_INACTIVE);
	}

	/* Configure Cols as Input with Pull-Up */
	for (int i = 0; i < 4; i++) {
		gpio_pin_configure(gpio_dev, cols[i], GPIO_INPUT | GPIO_PULL_UP);
	}

	printf("\n==========================================\n");
	printf("Keypad Task Communication Started\n");
	printf("==========================================\n");

	return 0;
}
