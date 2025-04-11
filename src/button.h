#ifndef BUTTON_H
#define BUTTON_H
#include <zephyr/drivers/gpio.h>

#define CUSTOM_BUTTON DT_ALIAS(custombutton)

static void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins);

int button_init(void);
#endif