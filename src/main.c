#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include "ble_service.h"

LOG_MODULE_REGISTER(WirelessDoorSensor, LOG_LEVEL_INF);

#define SLEEP_TIME_MS 100
#define CUSTOM_LED DT_ALIAS(customled)
#define CUSTOM_BUTTON DT_ALIAS(custombutton)
#define TEMP_SENSOR DT_ALIAS(temp)



static const struct gpio_dt_spec custom_led = GPIO_DT_SPEC_GET(CUSTOM_LED, gpios);
static const struct gpio_dt_spec custom_button = GPIO_DT_SPEC_GET(CUSTOM_BUTTON, gpios);
const struct device *const dev = DEVICE_DT_GET(TEMP_SENSOR);

static struct gpio_callback button_cb_data;

/* STEP 2.2 - Declare the structure for your custom data  */
typedef struct adv_sensor_data {
	int32_t temperature; 
	uint16_t humidity; 
} adv_sensor_data_type;

static adv_sensor_data_type adv_sensor_data = { 0, 0 };

int led_init(void)
{
	int ret;
	bool led_state = true;

	if (!gpio_is_ready_dt(&custom_led))
	{
		LOG_ERR("Custom led not ready");
		return -1;
	}
	ret = gpio_pin_configure_dt(&custom_led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0)
	{
		LOG_ERR("Custom led not ready (err %d)", ret);
		return ret;
	}
	return 0;
}

int button_init(void)
{
	if (!gpio_is_ready_dt(&custom_button))
	{
		return -1;
	}

	int ret = gpio_pin_configure_dt(&custom_button, GPIO_INT_EDGE_TO_ACTIVE);
	gpio_pin_interrupt_configure_dt(&custom_button, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret < 0)
	{
		return ret;
	}
	return 0;
}

void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	printf("Pressed\n");
    gpio_pin_toggle_dt(&custom_led);
	
}

int main(void)
{
	struct sensor_value temp_value;
	struct sensor_value humidity_value;
	__ASSERT(dev != NULL, "Failed to get device binding");
	__ASSERT(device_is_ready(dev), "Device %s is not ready", dev->name);
	printk("device is %p, name is %s\n", dev, dev->name);

	led_init();
	// device_is_ready(custom_led.port);
	// device_is_ready(custom_button.port);

	
	gpio_init_callback(&button_cb_data, button_pressed, BIT(custom_button.pin)); 	
	gpio_add_callback(custom_button.port, &button_cb_data);
	gpio_add_callback_dt(&custom_button, &button_cb_data);
	int err;

	ble_init();

	while (1)
	{
		err = sensor_sample_fetch(dev);
		if (err) {
			printk("sensor_sample_fetch failed ret %d\n", err);
			return;
		}
		err = sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp_value);
		if (err) {
			printk("Temperature sensor_channel_get failed ret %d\n", err);
			return;
		}
		err = sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &humidity_value);
		if (err) {
		printk("Humidity sensor_channel_get failed ret %d\n", err);
			return;
		}
		adv_sensor_data.humidity = humidity_value.val1 * 100 + humidity_value.val2;
		adv_sensor_data.temperature = temp_value.val1 * 100 + temp_value.val2;
		my_lbs_send_temp_notify((uint32_t)adv_sensor_data.temperature);
		my_lbs_send_humidity_notify(adv_sensor_data.humidity);
		printk("temp: %d.%d °C \t humidity is %d.%d%% \n", temp_value.val1,
					  temp_value.val2,humidity_value.val1, humidity_value.val2);
		update_advertisement_data();
		k_msleep(1500);
		
	}

	return 0;
}
