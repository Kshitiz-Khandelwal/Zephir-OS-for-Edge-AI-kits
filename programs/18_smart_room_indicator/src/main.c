/*
 * ╔══════════════════════════════════════════════════════════════╗
 * ║     Experiment 10: Smart Room Indicator (Events & Mutex)     ║
 * ╠══════════════════════════════════════════════════════════════╣
 * ║  WIRING                                                      ║
 * ║   Component            Pico GPIO                             ║
 * ║   ─────────            ─────────                             ║
 * ║   Light Sensor (LDR)   GP26 (ADC0)                           ║
 * ║   Touch Button         GP16                                  ║
 * ║   RGB Red              GP1                                   ║
 * ║   RGB Green            GP14                                  ║
 * ║   RGB Blue             GP20                                  ║
 * ╚══════════════════════════════════════════════════════════════╝
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/adc.h>
#include <stdio.h>

#define STACK_SIZE 1024
#define PRIORITY   7

/* ── ADC Configuration (LDR) ── */
#define ADC_NODE DT_NODELABEL(adc)
static const struct device *adc_dev = DEVICE_DT_GET(ADC_NODE);
#define ADC_RESOLUTION       12
#define ADC_GAIN             ADC_GAIN_1
#define ADC_REFERENCE        ADC_REF_INTERNAL
#define ADC_ACQUISITION_TIME ADC_ACQ_TIME_DEFAULT
#define LDR_CHANNEL          0

/* ── GPIO Configuration ── */
static const struct device *gpio0_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));

#define BUTTON_PIN 16
#define RED_PIN    1
#define GREEN_PIN  14
#define BLUE_PIN   20

/* ── Shared Variables & Mutex ── */
struct sensor_data_t {
    bool button_pressed;
    int16_t light_value;
};

struct sensor_data_t shared_sensors = {0};
K_MUTEX_DEFINE(sensor_mutex);

/* ─────────────────────────────────────────────────────────────
 * Task 1: Sensor Reading Task
 * ───────────────────────────────────────────────────────────── */
void sensor_task(void *a, void *b, void *c)
{
    struct adc_sequence sequence = {
        .channels    = BIT(LDR_CHANNEL),
        .buffer      = NULL,
        .buffer_size = sizeof(int16_t),
        .resolution  = ADC_RESOLUTION,
    };
    int16_t sample_buffer;

    while (1) {
        sequence.buffer = &sample_buffer;
        int ldr_val = 0;
        
        if (adc_read(adc_dev, &sequence) == 0) {
            ldr_val = sample_buffer;
        }

        bool pressed = gpio_pin_get(gpio0_dev, BUTTON_PIN) > 0;

        /* Protect shared variables */
        k_mutex_lock(&sensor_mutex, K_FOREVER);
        shared_sensors.button_pressed = pressed;
        shared_sensors.light_value = ldr_val;
        k_mutex_unlock(&sensor_mutex);

        k_sleep(K_MSEC(100));
    }
}

/* ─────────────────────────────────────────────────────────────
 * Task 2: Indicator Task (Logic)
 * ───────────────────────────────────────────────────────────── */
void indicator_task(void *a, void *b, void *c)
{
    while (1) {
        bool pressed;
        int16_t light;

        k_mutex_lock(&sensor_mutex, K_FOREVER);
        pressed = shared_sensors.button_pressed;
        light = shared_sensors.light_value;
        k_mutex_unlock(&sensor_mutex);

        /* Note: LDR in Zephyr with 12-bit ADC gives 0-4095.
         * The Python code used 16-bit (0-65535). 
         * 56000 in 16-bit is ~3500 in 12-bit (Dark).
         * 10000 in 16-bit is ~625 in 12-bit (Bright).
         */
        
        if (light > 3500) { /* Dark Room */
            if (pressed) {
                printf("Dark Room - Touch Detected | RED LED ON | LDR = %d\n", light);
                gpio_pin_set(gpio0_dev, RED_PIN, 0);   /* Active Low */
                gpio_pin_set(gpio0_dev, GREEN_PIN, 1);
                gpio_pin_set(gpio0_dev, BLUE_PIN, 1);
            } else {
                printf("Dark Room - Waiting for Touch | LED OFF | LDR = %d\n", light);
                gpio_pin_set(gpio0_dev, RED_PIN, 1);
                gpio_pin_set(gpio0_dev, GREEN_PIN, 1);
                gpio_pin_set(gpio0_dev, BLUE_PIN, 1);
            }
        } else if (light < 625) { /* Bright Room */
            printf("Bright Room | LED OFF | LDR = %d\n", light);
            gpio_pin_set(gpio0_dev, RED_PIN, 1);
            gpio_pin_set(gpio0_dev, GREEN_PIN, 1);
            gpio_pin_set(gpio0_dev, BLUE_PIN, 1);
        } else { /* Normal Room */
            printf("Normal Room | LED OFF | LDR = %d\n", light);
            gpio_pin_set(gpio0_dev, RED_PIN, 1);
            gpio_pin_set(gpio0_dev, GREEN_PIN, 1);
            gpio_pin_set(gpio0_dev, BLUE_PIN, 1);
        }

        k_sleep(K_MSEC(200));
    }
}

/* Define Thread Stacks */
K_THREAD_STACK_DEFINE(sensor_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(indicator_stack, STACK_SIZE);
struct k_thread thread_sensor;
struct k_thread thread_indicator;

/* ─────────────────────────────────────────────────────────────
 * Main Function Setup
 * ───────────────────────────────────────────────────────────── */
int main(void)
{
    /* Allow USB CDC ACM time to enumerate */
    k_sleep(K_MSEC(2000));
    
    printf("\n==========================================\n");
    printf(" Smart Room Indicator Started\n");
    printf("==========================================\n");

    if (!device_is_ready(gpio0_dev)) {
        printf("GPIO device not ready!\n");
        return -1;
    }
    if (!device_is_ready(adc_dev)) {
        printf("ADC device not ready!\n");
        return -1;
    }

    /* Configure GPIO */
    gpio_pin_configure(gpio0_dev, BUTTON_PIN, GPIO_INPUT | GPIO_PULL_DOWN);
    
    /* RGB Active LOW, set 1 to turn OFF */
    gpio_pin_configure(gpio0_dev, RED_PIN, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure(gpio0_dev, GREEN_PIN, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure(gpio0_dev, BLUE_PIN, GPIO_OUTPUT_ACTIVE);
    gpio_pin_set(gpio0_dev, RED_PIN, 1);
    gpio_pin_set(gpio0_dev, GREEN_PIN, 1);
    gpio_pin_set(gpio0_dev, BLUE_PIN, 1);

    /* Configure ADC Channel */
    struct adc_channel_cfg channel_cfg = {
        .gain = ADC_GAIN,
        .reference = ADC_REFERENCE,
        .acquisition_time = ADC_ACQUISITION_TIME,
        .channel_id = LDR_CHANNEL,
        .differential = 0
    };
    adc_channel_setup(adc_dev, &channel_cfg);

    /* Start Threads dynamically to avoid macro bugs */
    k_thread_create(&thread_sensor, sensor_stack, K_THREAD_STACK_SIZEOF(sensor_stack),
                    sensor_task, NULL, NULL, NULL, PRIORITY, 0, K_NO_WAIT);
    
    k_thread_create(&thread_indicator, indicator_stack, K_THREAD_STACK_SIZEOF(indicator_stack),
                    indicator_task, NULL, NULL, NULL, PRIORITY, 0, K_NO_WAIT);

    return 0;
}
