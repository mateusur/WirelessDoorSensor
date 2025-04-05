#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>

LOG_MODULE_REGISTER(Lesson2_Exercise1, LOG_LEVEL_INF);

#define SLEEP_TIME_MS 100
#define CUSTOM_LED DT_ALIAS(customled)
#define CUSTOM_BUTTON DT_ALIAS(custombutton)
#define TEMP_SENSOR DT_ALIAS(temp)

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

static const struct gpio_dt_spec custom_led = GPIO_DT_SPEC_GET(CUSTOM_LED, gpios);
static const struct gpio_dt_spec custom_button = GPIO_DT_SPEC_GET(CUSTOM_BUTTON, gpios);
const struct device *const dev = DEVICE_DT_GET(TEMP_SENSOR);

static struct gpio_callback button_cb_data;

static const struct bt_data ad[] = {
	/* STEP 4.1.2 - Set the advertising flags */
	BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
	/* STEP 4.1.3 - Set the advertising packet data  */
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static unsigned char url_data[] = {0x17, '/', '/', 'a', 'c', 'a', 'd', 'e', 'm', 'y', '.',
	'n', 'o', 'r', 'd', 'i', 'c', 's', 'e', 'm', 'i', '.',
	'c', 'o', 'm'};

static const struct bt_data sd[] = {
	/* 4.2.3 Include the URL data in the scan response packet*/
	BT_DATA(BT_DATA_URI, url_data, sizeof(url_data)),
};

bool led_init(void)
{
	int ret;
	bool led_state = true;

	if (!gpio_is_ready_dt(&custom_led))
	{
		return 0;
	}
	if (!gpio_is_ready_dt(&custom_button))
	{
		return 0;
	}

	ret = gpio_pin_configure_dt(&custom_led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0)
	{
		return 0;
	}
	ret = gpio_pin_configure_dt(&custom_button, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret < 0)
	{
		return 0;
	}
	while (1)
	{
		ret = gpio_pin_toggle_dt(&custom_led);
		if (ret < 0)
		{
			return 0;
		}

		led_state = !led_state;
		printf("LED state: %s\n", led_state ? "ON" : "OFF");
		k_msleep(SLEEP_TIME_MS);
	}
}

void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	printf("Pressed\n");
    gpio_pin_toggle_dt(&custom_led);
	
}

int main(void)
{
	// const struct device *const dev = DEVICE_DT_GET_ANY(sensirion_sht22);
	struct sensor_value temp_value;
	struct sensor_value humidity_value;
	__ASSERT(dev != NULL, "Failed to get device binding");
	__ASSERT(device_is_ready(dev), "Device %s is not ready", dev->name);
	printk("device is %p, name is %s\n", dev, dev->name);

	// do_main(dev);

	// device_is_ready(custom_led.port);
	// device_is_ready(custom_button.port);

	// gpio_pin_configure_dt(&custom_led, GPIO_OUTPUT_ACTIVE);
	// gpio_pin_configure_dt(&custom_button, GPIO_INPUT);
	
	// gpio_pin_interrupt_configure_dt(&custom_button, GPIO_INT_EDGE_TO_ACTIVE);
	// gpio_init_callback(&button_cb_data, button_pressed, BIT(custom_button.pin)); 	
	// gpio_add_callback(custom_button.port, &button_cb_data);
	// gpio_add_callback_dt(&custom_button, &button_cb_data);
	int err;
	LOG_INF("Error while fetching1");
	LOG_ERR("Error while fetching2");

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
		printk("temp: %d.%d °C \t humidity is %d.%d%% \n", temp_value.val1,
					  temp_value.val2,humidity_value.val1, humidity_value.val2);
		k_msleep(1000);
	}
	// err = bt_enable(NULL);
	// if (err)
	// {
	// 	LOG_ERR("Bluetooth init failed (err %d)\n", err);
	// 	return -1;
	// }
	// LOG_INF("Bluetooth initialized\n");

	// err = bt_le_adv_start(BT_LE_ADV_NCONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	// if (err)
	// {
	// 	LOG_ERR("Advertising failed to start (err %d)\n", err);
	// 	return -1;
	// }
	// LOG_INF("Advertising successfully started\n");

	

	return 0;
}
