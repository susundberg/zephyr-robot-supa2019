#include <zephyr.h>
#include <devicetree.h>

#include <drivers/gpio.h>
#define SUPA_MODULE "sla"
#include "../main.h"
#include "sensor_lawn.h"
#include "../utils/change_filter.h"

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

static struct device *LOCAL_dev_capin;
static struct device *LOCAL_dev_capout;

static ChangeFilter LOCAL_changle_filter = { .value_th = 10 };
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

static u32_t read_single_value()
{
	// Empty charge
	gpio_pin_set(LOCAL_dev_capout, GPIO_PIN(LEDS_CAP_OUT), 0);
	gpio_pin_set(LOCAL_dev_capin, GPIO_PIN(KEYS_CAP_IN), 0);
	k_msgq_purge(&LOCAL_queue);
	k_msleep(SENSOR_LAWN_SAMPLE_WAIT_MS);

	// uint32_t pin_value = gpio_pin_get(LOCAL_dev_capin, GPIO_PIN(KEYS_CAP_IN));

	// if (pin_value != 0)
	// {
	// 	LOG_WRN("Pin value is still %d, retry", pin_value);
	// 	continue;
	// }

	// Start charge
	gpio_pin_set(LOCAL_dev_capin, GPIO_PIN(KEYS_CAP_IN), 1);
	u32_t start_time = k_cycle_get_32();

	gpio_pin_set(LOCAL_dev_capout, GPIO_PIN(LEDS_CAP_OUT), 1);

	u32_t end_time = 0;
	int32_t ret = k_msgq_get(&LOCAL_queue, &end_time, K_MSEC(SENSOR_LAWN_SAMPLE_MAX_LOAD_MS));
	gpio_pin_set(LOCAL_dev_capout, GPIO_PIN(LEDS_CAP_OUT), 0);
	gpio_pin_set(LOCAL_dev_capin, GPIO_PIN(KEYS_CAP_IN), 0);

	if (ret == 0)
	{
		// Got new message
		if (end_time <= start_time)
		{
			LOG_INF("Overflow!");
			return 0;
		}

		u32_t dtime = (u32_t)(k_cyc_to_ns_floor64(end_time - start_time));
		dtime = dtime / 1000;
		return dtime;
	}
	else
	{
		LOG_INF("Did not observe signal!");
		return 0;
	}
}

static u32_t read_sensor_value()
{
	u32_t value_min = 1 << 30;
	u32_t value_max = 0;
	for ( s32_t loop = 0; loop < SENSOR_LAWN_SAMPLE_N ; loop ++ )
	{
		u32_t value = read_single_value();

		if (value == 0)
			continue;

		value_min = MIN( value_min, value );
		value_max = MAX( value_max, value );
		
	}
	if ( value_min > value_max )
	{
		FATAL_ERROR("Lawn sensor is not working!");
		return 0;
	}
	return (value_max);
}

static void sensor_lawn_main(void)
{
	LOG_INF("Thread started!");

	GPIO_CONFIGURE(&LOCAL_dev_capout, LEDS_CAP_OUT, GPIO_OUTPUT_ACTIVE | GPIO_ACTIVE_HIGH | GPIO_OUTPUT);
	GPIO_CONFIGURE(&LOCAL_dev_capin, KEYS_CAP_IN, GPIO_INPUT | GPIO_ACTIVE_HIGH | GPIO_OUTPUT | GPIO_OPEN_DRAIN);

	k_msgq_init(&LOCAL_queue, (char *)LOCAL_queue_buffer, 4, LOCAL_queue_n);

	gpio_init_callback(&LOCAL_ISR_signal, signal_isr, BIT(GPIO_PIN(KEYS_CAP_IN)));
	RET_CHECK(gpio_add_callback(LOCAL_dev_capin, &LOCAL_ISR_signal));
	RET_CHECK(gpio_pin_interrupt_configure(LOCAL_dev_capin, GPIO_PIN(KEYS_CAP_IN), GPIO_INT_EDGE_RISING));


	u32_t value_filt = read_sensor_value();

	

	for (;;)
	{
		u32_t value = read_sensor_value();

		value_filt = (900*value_filt + value*100) / 1000;
		// LOG_INF("Got %d %d", value, value_filt );

		if ( change_filtered( value_filt, &LOCAL_changle_filter ) )
		{
			LOG_INF("Lawn sensor: %d", value_filt );
		}

		k_msleep(SENSOR_LAWN_WAIT_BETWEEN_MS);
	}

}

K_THREAD_DEFINE(sensor_lawn, 1024, sensor_lawn_main, NULL, NULL, NULL,
				OS_DEFAULT_PRIORITY, 0, 0);
