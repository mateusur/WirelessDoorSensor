#ifndef SHT_SENSOR_H
#define SHT_SENSOR_H

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/gpio.h>

#define TEMP_SENSOR DT_ALIAS(temp)
const struct device *const dev = DEVICE_DT_GET(TEMP_SENSOR);

int sht_sensor_init(void);
sensor_value sht_sensor_get_temp_c(void);
sensor_value sht_sensor_get_humidity(void);

#endif