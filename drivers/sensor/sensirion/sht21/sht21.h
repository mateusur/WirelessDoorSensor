#ifndef ZEPHYR_DRIVERS_SENSOR_SHT21_SHT21_H_
#define ZEPHYR_DRIVERS_SENSOR_SHT21_SHT21_H_

#include <zephyr/device.h>
#include <zephyr/sys/util.h>

#define TRIGGER_TEMP_MEASUREMENT_HOLD       0xE3
#define TRIGGER_RH_MEASUREMENT_HOLD         0xE5     
#define TRIGGER_TEMP_MEASUREMENT_NO_HOLD    0xF3
#define TRIGGER_RH_MEASUREMENT_NO_HOLD      0xF5
#define WRITE_USER                          0xE6
#define READ_USER                           0xE7
#define SOFT_RESET                          0xFE



#endif