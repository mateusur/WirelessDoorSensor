#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/addr.h>

#include <zephyr/logging/log.h>
#include "ble_service.h"
LOG_MODULE_REGISTER(BleService, LOG_LEVEL_INF);

struct bt_conn *my_conn = NULL;

static bool notify_temperature_enabled;
static bool notify_humidity_enabled;


static struct bt_gatt_indicate_params ind_params;

void on_connected(struct bt_conn *conn, uint8_t err)
{
    if (err) {
        LOG_ERR("Connection error %d", err);
        return;
    }
    LOG_INF("Connected");
    my_conn = bt_conn_ref(conn);

	// gpio_pin_toggle_dt(&custom_led);
	// gpio_pin_set_dt(&custom_led,1);
}

void on_disconnected(struct bt_conn *conn, uint8_t reason)
{
    LOG_INF("Disconnected. Reason %d", reason);
    bt_conn_unref(my_conn);

	// gpio_pin_toggle_dt(&custom_led);
	// gpio_pin_set_dt(&custom_led,0);

}

struct bt_conn_cb connection_callbacks = {
    .connected = on_connected,
    .disconnected = on_disconnected,
};
static const struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM(
	(BT_LE_ADV_OPT_CONNECTABLE |
	 BT_LE_ADV_OPT_USE_IDENTITY), /* Connectable advertising and use identity address */
	BT_GAP_ADV_FAST_INT_MIN_1, /* 0x30 units, 48 units, 30ms */
	BT_GAP_ADV_FAST_INT_MAX_1, /* 0x60 units, 96 units, 60ms */
	NULL); /* Set to NULL for undirected advertising */

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};
BT_GATT_SERVICE_DEFINE(
    custom_service,
	BT_GATT_PRIMARY_SERVICE(BT_UUID_MY_SERVICE),
	BT_GATT_CHARACTERISTIC(BT_UUID_MY_TEMPERATURE, BT_GATT_CHRC_READ|BT_GATT_CHRC_NOTIFY,BT_GATT_PERM_READ, my_read_temperature, NULL, NULL),
	BT_GATT_CCC(mylbsbc_ccc_temperature_changed,BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	BT_GATT_CHARACTERISTIC(BT_UUID_MY_HUMIDITY, BT_GATT_CHRC_READ|BT_GATT_CHRC_NOTIFY,BT_GATT_PERM_READ, my_read_humidity, NULL, NULL),
	BT_GATT_CCC(mylbsbc_ccc_humidity_changed,BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

);
ssize_t my_read_temperature(struct bt_conn *conn,const struct bt_gatt_attr *attr, void *buf,uint16_t len, uint16_t offset,int32_t temperature){
    return bt_gatt_attr_read(conn, attr, buf, len, offset,&temperature,sizeof(temperature));
}

ssize_t my_read_humidity(struct bt_conn *conn,const struct bt_gatt_attr *attr, void *buf,uint16_t len, uint16_t offset,uint16_t humidity){
    return bt_gatt_attr_read(conn, attr, buf, len, offset,&humidity,sizeof(humidity));
}

static void mylbsbc_ccc_temperature_changed(const struct bt_gatt_attr *attr,uint16_t value){
	notify_temperature_enabled = (value == BT_GATT_CCC_NOTIFY);
	LOG_INF("Temperature notification %s", notify_temperature_enabled ? "enabled" : "disabled");
}

static void mylbsbc_ccc_humidity_changed(const struct bt_gatt_attr *attr,uint16_t value){
	notify_humidity_enabled = (value == BT_GATT_CCC_NOTIFY);
	LOG_INF("Humidity notification %s", notify_humidity_enabled ? "enabled" : "disabled");
}



int my_lbs_send_temp_notify(uint32_t temp_value)
{
	if (!notify_temperature_enabled) {
		LOG_ERR("notify_temperature_enabled is not enabled/set");
		return -EACCES;
	}

	return bt_gatt_notify(NULL, &custom_service.attrs[2],&temp_value,sizeof(temp_value));
}

int my_lbs_send_humidity_notify(uint16_t humidity_value){
	if (!notify_humidity_enabled) {
		LOG_ERR("notify_humidity_enabled is not enabled/set");
		return -EACCES;
	}

	return bt_gatt_notify(NULL, &custom_service.attrs[4],&humidity_value,sizeof(humidity_value));
}
int ble_init(){
    int err = bt_conn_cb_register(&connection_callbacks);
    if (err) {
		LOG_ERR("Connection callback register failed (err %d)", err);
        return err;
    }
    err = bt_enable(NULL);
	if (err)
	{
		LOG_ERR("Bluetooth init failed (err %d)\n", err);
		return err;
	}
	err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), NULL ,0);
	if (err)
	{
		LOG_ERR("Advertising failed to start (err %d)\n", err);
		return err;
	}
    return 0;
}

void update_advertisement_data(){
    bt_le_adv_update_data(ad, ARRAY_SIZE(ad), NULL,0);
}