#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/addr.h>

LOG_MODULE_REGISTER(WirelessDoorSensor, LOG_LEVEL_INF);

#define SLEEP_TIME_MS 100
#define CUSTOM_LED DT_ALIAS(customled)
#define CUSTOM_BUTTON DT_ALIAS(custombutton)
#define TEMP_SENSOR DT_ALIAS(temp)

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

#define BT_UUID_MY_SERVICE_VALUE BT_UUID_128_ENCODE(0xc504356a,0x8d14,0x480b,0x8fca,0x09800799ccf1)
#define BT_UUID_MY_SERVICE BT_UUID_DECLARE_128(BT_UUID_MY_SERVICE_VALUE)

#define BT_UUID_MY_TEMPERATURE_VALUE BT_UUID_128_ENCODE(0xc504356a,0x8d14,0x480b,0x8fca,0x09800799ccf2)
#define BT_UUID_MY_TEMPERATURE BT_UUID_DECLARE_128(BT_UUID_MY_TEMPERATURE_VALUE)

#define BT_UUID_MY_HUMIDITY_VALUE BT_UUID_128_ENCODE(0xc504356a,0x8d14,0x480b,0x8fca,0x09800799ccf3)
#define BT_UUID_MY_HUMIDITY BT_UUID_DECLARE_128(BT_UUID_MY_HUMIDITY_VALUE)

static const struct gpio_dt_spec custom_led = GPIO_DT_SPEC_GET(CUSTOM_LED, gpios);
static const struct gpio_dt_spec custom_button = GPIO_DT_SPEC_GET(CUSTOM_BUTTON, gpios);
const struct device *const dev = DEVICE_DT_GET(TEMP_SENSOR);

static struct gpio_callback button_cb_data;

struct bt_conn *my_conn = NULL;

static bool notify_mytemperature_enabled;

static struct bt_gatt_indicate_params ind_params;


static const struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM(
	(BT_LE_ADV_OPT_CONNECTABLE |
	 BT_LE_ADV_OPT_USE_IDENTITY), /* Connectable advertising and use identity address */
	BT_GAP_ADV_FAST_INT_MIN_1, /* 0x30 units, 48 units, 30ms */
	BT_GAP_ADV_FAST_INT_MAX_1, /* 0x60 units, 96 units, 60ms */
	NULL); /* Set to NULL for undirected advertising */



void on_connected(struct bt_conn *conn, uint8_t err)
{
    if (err) {
        LOG_ERR("Connection error %d", err);
        return;
    }
    LOG_INF("Connected");
    my_conn = bt_conn_ref(conn);

    /* STEP 3.2  Turn the connection status LED on */
	gpio_pin_toggle_dt(&custom_led);
	// gpio_pin_set_dt(&custom_led,1);
}

void on_disconnected(struct bt_conn *conn, uint8_t reason)
{
    LOG_INF("Disconnected. Reason %d", reason);
    bt_conn_unref(my_conn);

    /* STEP 3.3  Turn the connection status LED off */
	gpio_pin_toggle_dt(&custom_led);
	// gpio_pin_set_dt(&custom_led,0);

}

struct bt_conn_cb connection_callbacks = {
    .connected              = on_connected,
    .disconnected           = on_disconnected,
};

/* STEP 2.2 - Declare the structure for your custom data  */
typedef struct adv_sensor_data {
	int32_t temperature; 
	uint16_t humidity; 
} adv_sensor_data_type;

static adv_sensor_data_type adv_sensor_data = { 0, 0 };

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

ssize_t my_read_temperature(struct bt_conn *conn,const struct bt_gatt_attr *attr, void *buf,uint16_t len, uint16_t offset){
    return bt_gatt_attr_read(conn, attr, buf, len, offset,&adv_sensor_data.temperature,sizeof(adv_sensor_data.temperature));
}

ssize_t my_read_humidity(struct bt_conn *conn,const struct bt_gatt_attr *attr, void *buf,uint16_t len, uint16_t offset){
    return bt_gatt_attr_read(conn, attr, buf, len, offset,&adv_sensor_data.humidity,sizeof(adv_sensor_data.humidity));
}

static void mylbsbc_ccc_temperature_changed(const struct bt_gatt_attr *attr,uint16_t value){
	notify_mytemperature_enabled = (value == BT_GATT_CCC_NOTIFY);
	LOG_INF("Notification %s", notify_mytemperature_enabled ? "enabled" : "disabled");
}


BT_GATT_SERVICE_DEFINE(
    custom_service,
	BT_GATT_PRIMARY_SERVICE(BT_UUID_MY_SERVICE),
	BT_GATT_CHARACTERISTIC(BT_UUID_MY_TEMPERATURE, BT_GATT_CHRC_READ|BT_GATT_CHRC_NOTIFY,BT_GATT_PERM_READ, my_read_temperature, NULL, NULL),
	BT_GATT_CCC(mylbsbc_ccc_temperature_changed,BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	BT_GATT_CHARACTERISTIC(BT_UUID_MY_HUMIDITY, BT_GATT_CHRC_READ,BT_GATT_PERM_READ, my_read_humidity, NULL, NULL),
);

int my_lbs_send_temp_notify(uint32_t temp_value)
{
	if (!notify_mytemperature_enabled) {
		LOG_ERR("notify_mytemperature_enabled is not enabled/set");
		return -EACCES;
	}

	return bt_gatt_notify(NULL, &custom_service.attrs[2],&temp_value,sizeof(temp_value));
}
bool led_init(void)
{
	int ret;
	bool led_state = true;

	if (!gpio_is_ready_dt(&custom_led))
	{
		LOG_ERR("Custom led not ready");
		return 0;
	}
	// if (!gpio_is_ready_dt(&custom_button))
	// {
	// 	return 0;
	// }

	ret = gpio_pin_configure_dt(&custom_led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0)
	{
		LOG_ERR("Custom led not ready (err %d)", ret);
		return 0;
	}
	// ret = gpio_pin_configure_dt(&custom_button, GPIO_INT_EDGE_TO_ACTIVE);
	// if (ret < 0)
	// {
	// 	return 0;
	// }
	// while (1)
	// {
	// 	ret = gpio_pin_toggle_dt(&custom_led);
	// 	if (ret < 0)
	// 	{
	// 		return 0;
	// 	}

	// 	led_state = !led_state;
	// 	printf("LED state: %s\n", led_state ? "ON" : "OFF");
	// 	k_msleep(SLEEP_TIME_MS);
	// }
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

	led_init();
	// device_is_ready(custom_led.port);
	// device_is_ready(custom_button.port);

	// gpio_pin_configure_dt(&custom_led, GPIO_OUTPUT_ACTIVE);
	// gpio_pin_configure_dt(&custom_button, GPIO_INPUT);
	
	// gpio_pin_interrupt_configure_dt(&custom_button, GPIO_INT_EDGE_TO_ACTIVE);
	// gpio_init_callback(&button_cb_data, button_pressed, BIT(custom_button.pin)); 	
	// gpio_add_callback(custom_button.port, &button_cb_data);
	// gpio_add_callback_dt(&custom_button, &button_cb_data);
	int err;

	err = bt_conn_cb_register(&connection_callbacks);
	if (err) {
		LOG_ERR("Connection callback register failed (err %d)", err);
    }
	err = bt_enable(NULL);
	if (err)
	{
		LOG_ERR("Bluetooth init failed (err %d)\n", err);
		return -1;
	}
	LOG_INF("Bluetooth initialized\n");
	bt_conn_cb_register(&connection_callbacks);

	err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), NULL ,0);
	if (err)
	{
		LOG_ERR("Advertising failed to start (err %d)\n", err);
		return -1;
	}
	LOG_INF("Advertising successfully started\n");

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
		printk("temp: %d.%d °C \t humidity is %d.%d%% \n", temp_value.val1,
					  temp_value.val2,humidity_value.val1, humidity_value.val2);
		bt_le_adv_update_data(ad, ARRAY_SIZE(ad), NULL,0);
		k_msleep(1000);
		
	}

	return 0;
}
