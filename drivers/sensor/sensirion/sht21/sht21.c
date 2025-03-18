#define DT_DRV_COMPAT sensirion_sht21
#if DT_NUM_INST_STATUS_OKAY(DT_DRV_COMPAT) == 0
#warning "SHT21 driver enabled without any devices"
#endif

#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include "sht21.h"

struct sht21_data {
	uint16_t temp;
    uint16_t humidity
    uint8_t config_req;
};

struct sht21_config {
	const struct i2c_dt_spec bus;
    uint8_t crc;
};

__subsystem struct sensor_driver_api {
    sensor_sample_fetch_t sample_fetch;
    sensor_channel_get_t channel_get;
};
int sample_fetch(const struct device *dev, enum sensor_channel chan){}
int channel_get(const struct device *dev, enum sensor_channel chan, struct sensor_value *val){}
static const struct sensor_driver_api sht21_api ={
    .sample_fetch = &sample_fetch,
    .channel_get = &channel_get,
};