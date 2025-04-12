#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include "LED.h"
LOG_MODULE_REGISTER(LEDModule, LOG_LEVEL_INF);

static const struct gpio_dt_spec custom_led = GPIO_DT_SPEC_GET(CUSTOM_LED, gpios);

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