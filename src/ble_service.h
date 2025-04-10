#ifndef BLE_SERVICE_H
#define BLE_SERVICE_H
#include <stdio.h>
#include <zephyr/kernel.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/addr.h>

#define BT_UUID_MY_SERVICE_VALUE BT_UUID_128_ENCODE(0xc504356a,0x8d14,0x480b,0x8fca,0x09800799ccf1)
#define BT_UUID_MY_SERVICE BT_UUID_DECLARE_128(BT_UUID_MY_SERVICE_VALUE)

#define BT_UUID_MY_TEMPERATURE_VALUE BT_UUID_128_ENCODE(0xc504356a,0x8d14,0x480b,0x8fca,0x09800799ccf2)
#define BT_UUID_MY_TEMPERATURE BT_UUID_DECLARE_128(BT_UUID_MY_TEMPERATURE_VALUE)

#define BT_UUID_MY_HUMIDITY_VALUE BT_UUID_128_ENCODE(0xc504356a,0x8d14,0x480b,0x8fca,0x09800799ccf3)
#define BT_UUID_MY_HUMIDITY BT_UUID_DECLARE_128(BT_UUID_MY_HUMIDITY_VALUE)

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

ssize_t my_read_temperature(struct bt_conn *conn,const struct bt_gatt_attr *attr, void *buf,uint16_t len, uint16_t offset,int32_t temperature);
ssize_t my_read_humidity(struct bt_conn *conn,const struct bt_gatt_attr *attr, void *buf,uint16_t len, uint16_t offset,uint16_t humidity);

static void mylbsbc_ccc_temperature_changed(const struct bt_gatt_attr *attr,uint16_t value);
static void mylbsbc_ccc_humidity_changed(const struct bt_gatt_attr *attr,uint16_t value);

int ble_init();
void update_advertisement_data();
void on_connected(struct bt_conn *conn, uint8_t err);
void on_disconnected(struct bt_conn *conn, uint8_t reason);


#endif