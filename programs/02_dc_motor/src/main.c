/*
 * ╔══════════════════════════════════════════════════════════════╗
 * ║        DC Motor Control — Zephyr RTOS on Pico 2             ║
 * ╠══════════════════════════════════════════════════════════════╣
 * ║  HARDWARE REQUIRED                                           ║
 * ║  • Raspberry Pi Pico 2                                       ║
 * ║  • L298N Dual Motor Driver Module                            ║
 * ║  • DC Motor (5V–12V)                                         ║
 * ║  • External power supply (battery / adapter)                 ║
 * ╠══════════════════════════════════════════════════════════════╣
 * ║  WIRING DIAGRAM                                              ║
 * ║                                                              ║
 * ║   Pico 2          L298N Module                               ║
 * ║   ───────         ────────────────────────                   ║
 * ║   GP2      ────►  IN1   (direction A)                        ║
 * ║   GP3      ────►  IN2   (direction B)                        ║
 * ║   GP4      ────►  ENA   (speed via PWM)  ← remove jumper!   ║
 * ║   GND      ────►  GND   (COMMON GROUND — REQUIRED!)          ║
 * ║                                                              ║
 * ║   L298N           Motor                                      ║
 * ║   ───────         ──────                                     ║
 * ║   OUT1     ────►  Motor Terminal +                           ║
 * ║   OUT2     ────►  Motor Terminal −                           ║
 * ║                                                              ║
 * ║   External Power  L298N                                      ║
 * ║   ─────────────   ──────                                     ║
 * ║   + (6–12V)  ──►  12V pin                                    ║
 * ║   − (GND)    ──►  GND pin                                    ║
 * ╠══════════════════════════════════════════════════════════════╣
 * ║  IMPORTANT NOTES                                             ║
 * ║  • Remove the ENA jumper from L298N before wiring GP4→ENA   ║
 * ║  • Always connect Pico GND to L298N GND (common ground)     ║
 * ║  • Do NOT power motor from Pico's 5V pin (draw too high)    ║
 * ╚══════════════════════════════════════════════════════════════╝
 *
 *  MOTOR SEQUENCE:
 *    1. Forward  50%  for 3 sec
 *    2. Ramp     50→100% over 1.5 sec
 *    3. Stop          for 1 sec
 *    4. Backward 75%  for 3 sec
 *    5. Stop          for 1 sec
 *    6. Repeat forever
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(motor, LOG_LEVEL_INF);

/* ── Pin assignments ──────────────────────────────────────────── */
#define IN1_PIN  2   /* GP2 — L298N IN1 (direction) */
#define IN2_PIN  3   /* GP3 — L298N IN2 (direction) */
                     /* GP4 — L298N ENA (PWM speed) via overlay  */

/* ── PWM spec from device tree alias ─────────────────────────── */
static const struct pwm_dt_spec motor_pwm =
	PWM_DT_SPEC_GET(DT_ALIAS(motor_pwm));

#define PWM_PERIOD_NS  PWM_MSEC(1)   /* 1 kHz PWM */

/* ── GPIO device (GP2 and GP3 are on gpio0) ──────────────────── */
static const struct device *gpio_dev;

/* ── Motor direction type ─────────────────────────────────────── */
typedef enum {
	MOTOR_FORWARD,
	MOTOR_BACKWARD,
	MOTOR_STOP
} motor_dir_t;

/* ─────────────────────────────────────────────────────────────────
 * motor_set()  —  Set direction and speed
 *   dir       : MOTOR_FORWARD | MOTOR_BACKWARD | MOTOR_STOP
 *   speed_pct : 0 (stopped) to 100 (full speed)
 * ───────────────────────────────────────────────────────────────── */
void motor_set(motor_dir_t dir, uint8_t speed_pct)
{
	if (speed_pct > 100) {
		speed_pct = 100;
	}

	/* Convert percentage to PWM pulse width */
	uint32_t pulse_ns =
		(uint32_t)((uint64_t)PWM_PERIOD_NS * speed_pct / 100U);

	switch (dir) {
	case MOTOR_FORWARD:
		gpio_pin_set(gpio_dev, IN1_PIN, 1);
		gpio_pin_set(gpio_dev, IN2_PIN, 0);
		LOG_INF(">> FORWARD  %3d%%", speed_pct);
		break;

	case MOTOR_BACKWARD:
		gpio_pin_set(gpio_dev, IN1_PIN, 0);
		gpio_pin_set(gpio_dev, IN2_PIN, 1);
		LOG_INF("<< BACKWARD %3d%%", speed_pct);
		break;

	case MOTOR_STOP:
	default:
		gpio_pin_set(gpio_dev, IN1_PIN, 0);
		gpio_pin_set(gpio_dev, IN2_PIN, 0);
		pulse_ns = 0;
		LOG_INF("|| STOP");
		break;
	}

	pwm_set_dt(&motor_pwm, PWM_PERIOD_NS, pulse_ns);
}

/* ── Main ─────────────────────────────────────────────────────── */
int main(void)
{
	LOG_INF("============================================");
	LOG_INF("   DC Motor Control — Zephyr RTOS v4.4    ");
	LOG_INF("============================================");

	/* Verify PWM is ready */
	if (!pwm_is_ready_dt(&motor_pwm)) {
		LOG_ERR("PWM not ready! Check board overlay.");
		return -ENODEV;
	}

	/* Get GPIO0 controller */
	gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));
	if (!device_is_ready(gpio_dev)) {
		LOG_ERR("GPIO0 not ready!");
		return -ENODEV;
	}

	/* Configure direction pins as digital outputs, start LOW */
	gpio_pin_configure(gpio_dev, IN1_PIN, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure(gpio_dev, IN2_PIN, GPIO_OUTPUT_INACTIVE);

	LOG_INF("Motor initialised. Starting sequence...");

	while (1) {
		/* ── Phase 1: Forward at 50% ─────────────── */
		motor_set(MOTOR_FORWARD, 50);
		k_sleep(K_SECONDS(3));

		/* ── Phase 2: Ramp up 50% → 100% ─────────── */
		LOG_INF("Ramping speed up...");
		for (int s = 50; s <= 100; s += 10) {
			motor_set(MOTOR_FORWARD, (uint8_t)s);
			k_sleep(K_MSEC(300));
		}

		/* ── Phase 3: Stop ───────────────────────── */
		motor_set(MOTOR_STOP, 0);
		k_sleep(K_SECONDS(1));

		/* ── Phase 4: Backward at 75% ────────────── */
		motor_set(MOTOR_BACKWARD, 75);
		k_sleep(K_SECONDS(3));

		/* ── Phase 5: Stop ───────────────────────── */
		motor_set(MOTOR_STOP, 0);
		k_sleep(K_SECONDS(1));
	}

	return 0;
}
