#include "sht_sensor.h"

LOG_MODULE_REGISTER(ShtSensor, LOG_LEVEL_INF);

typedef struct adv_sensor_data {
	int32_t temperature; 
	uint16_t humidity; 
} adv_sensor_data_type;

static adv_sensor_data_type adv_sensor_data = { 0, 0 };

int sht_sensor_init(void) {
	__ASSERT(dev != NULL, "Failed to get device binding");
	__ASSERT(device_is_ready(dev), "Device %s is not ready", dev->name);
	printk("device is %p, name is %s\n", dev, dev->name);

    return 0;
}

sensor_value sht_sensor_get_temp_c(void) {
    struct sensor_value temp_value;
    int err = sensor_sample_fetch(dev);
    err = sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp_value);
    return err;
}

sensor_value sht_sensor_get_humidity(void) {
	struct sensor_value humidity_value;
    int err = sensor_sample_fetch(dev);
    err = sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &humidity_value);
    return err;
}
