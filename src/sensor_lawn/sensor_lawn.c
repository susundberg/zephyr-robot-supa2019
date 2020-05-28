#include <zephyr.h>
#include <devicetree.h>

#include <drivers/gpio.h>
#define SUPA_MODULE "sla"
#include "../main.h"
#include "sensor_lawn.h"

LOG_MODULE_REGISTER(SLA);

#define LOCAL_queue_n 4
#define SENSOR_LAWN_SAMPLE_N 8
#define SENSOR_LAWN_SAMPLE_WAIT_MS 25
#define SENSOR_LAWN_SAMPLE_MAX_LOAD_MS 50
#define SENSOR_LAWN_WAIT_BETWEEN_MS 00
#define SENSOW_LAWN_EXPECTED_VALUE 1500

#define GPIO_PIN(name) (DT_GPIO_##name##_GPIOS_PIN)
#define GPIO_CONFIGURE(dev, name, flags)                                                          \
	{                                                                                             \
		get_and_check(dev, DT_GPIO_##name##_GPIOS_CONTROLLER, DT_GPIO_##name##_GPIOS_PIN, flags); \
	}

static struct device *dev_cap_in;

static struct k_msgq LOCAL_queue;
static u32_t LOCAL_queue_buffer[LOCAL_queue_n];

static struct gpio_callback LOCAL_ISR_signal;
static u32_t LOCAL_edges = 0;

static void signal_isr(struct device *gpiob, struct gpio_callback *cb, u32_t pins)
{
	(void)gpiob;
	(void)pins;
	(void)cb;
	u32_t end_time = k_cycle_get_32();

	k_msgq_put(&LOCAL_queue, &end_time, K_NO_WAIT);
	LOCAL_edges++;
}

void get_and_check(struct device **dev, const char *label, u32_t pin, uint32_t flags)
{

	*dev = device_get_binding(label);

	LOG_INF("Configure dev %s: x%04X", label, flags);
	if (*dev == NULL)
	{
		LOG_ERR("Cannot get device %s", label);
		__ASSERT_NO_MSG(0);
		return;
	}

	int32_t ret = gpio_pin_configure(*dev, pin, flags);

	if (ret < 0)
	{
		LOG_ERR("Configuration failed:%d", ret);
		// __ASSERT_NO_MSG(0);
		return;
	}
}

static void sensor_lawn_main(void)
{
	LOG_INF("Thread started!");

	struct device *dev_cap_out;

	GPIO_CONFIGURE(&dev_cap_out, LEDS_CAP_OUT, GPIO_OUTPUT_ACTIVE | GPIO_ACTIVE_HIGH | GPIO_OUTPUT);
	GPIO_CONFIGURE(&dev_cap_in, KEYS_CAP_IN, GPIO_INPUT | GPIO_ACTIVE_HIGH | GPIO_OUTPUT | GPIO_OPEN_DRAIN);

	k_msgq_init(&LOCAL_queue, (char *)LOCAL_queue_buffer, 4, LOCAL_queue_n);

	gpio_init_callback(&LOCAL_ISR_signal, signal_isr, BIT(GPIO_PIN(KEYS_CAP_IN)));
	RET_CHECK(gpio_add_callback(dev_cap_in, &LOCAL_ISR_signal));
	RET_CHECK(gpio_pin_interrupt_configure(dev_cap_in, GPIO_PIN(KEYS_CAP_IN), GPIO_INT_EDGE_RISING));

	//	GPIO_CONFIGURE(&dev_cap_in, KEYS_CAP_IN ,  GPIO_ACTIVE_HIGH| GPIO_OUTPUT  );

	u32_t mainloop = 0;

	//	u32_t data[SENSOR_LAWN_SAMPLE_N] = {0};
	u32_t val_avg = 0;
	u32_t val_min = 1 << 30;
	u32_t val_max = 0;
	u32_t val_filt = SENSOW_LAWN_EXPECTED_VALUE;
	for (;;)
	{
		// Empty charge
		gpio_pin_set(dev_cap_out, GPIO_PIN(LEDS_CAP_OUT), 0);
		gpio_pin_set(dev_cap_in, GPIO_PIN(KEYS_CAP_IN), 0);
		k_msgq_purge(&LOCAL_queue);
		k_msleep(SENSOR_LAWN_SAMPLE_WAIT_MS);

		// uint32_t pin_value = gpio_pin_get(dev_cap_in, GPIO_PIN(KEYS_CAP_IN));

		// if (pin_value != 0)
		// {
		// 	LOG_WRN("Pin value is still %d, retry", pin_value);
		// 	continue;
		// }

		// Start charge
		gpio_pin_set(dev_cap_in, GPIO_PIN(KEYS_CAP_IN), 1);
		u32_t start_time = k_cycle_get_32();

		gpio_pin_set(dev_cap_out, GPIO_PIN(LEDS_CAP_OUT), 1);

		u32_t end_time = 0;
		int32_t ret = k_msgq_get(&LOCAL_queue, &end_time, K_MSEC(SENSOR_LAWN_SAMPLE_MAX_LOAD_MS));
		gpio_pin_set(dev_cap_out, GPIO_PIN(LEDS_CAP_OUT), 0);
		gpio_pin_set(dev_cap_in, GPIO_PIN(KEYS_CAP_IN), 0);

		if (ret == 0)
		{
			// Got new message
			if (end_time <= start_time)
			{
				LOG_INF("Overflow!");
				continue;
			}

			u32_t dtime = (u32_t)(k_cyc_to_ns_floor64(end_time - start_time));
			dtime = dtime / 1000;

			val_avg += (dtime);
			//printk("%02d ", data[loop] / 1000);
			val_min = MIN(dtime, val_min);
			val_max = MAX(dtime, val_max);

			mainloop += 1;
		}
		else
		{
			LOG_INF("Did not observe signal!");
			continue;
		}

		if (mainloop >= SENSOR_LAWN_SAMPLE_N)
		{

			mainloop = 0;

			u32_t vavg = (val_avg / SENSOR_LAWN_SAMPLE_N);
			u32_t val_range  = val_max - val_min;
			val_filt = (900*val_filt + 100*val_range) / 1000;
			LOG_INF("Values: filt: %d - avg: %d min: %d max:%d r:%d", val_filt, vavg, val_min, val_max, val_range);

			k_msleep(SENSOR_LAWN_WAIT_BETWEEN_MS);

			

			val_avg = 0;
			val_min = 1 << 30;
			val_max = 0;

			LOCAL_edges = 0;
		}
	}
}

K_THREAD_DEFINE(sensor_lawn, 1024, sensor_lawn_main, NULL, NULL, NULL,
				OS_DEFAULT_PRIORITY, 0, 0);
