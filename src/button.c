#include "button.h"

static const struct gpio_dt_spec custom_button = GPIO_DT_SPEC_GET(CUSTOM_BUTTON, gpios);
static struct gpio_callback button_cb_data;

static void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	// printf("Pressed\n");
    // led_toggle();
    // gpio_pin_toggle_dt(&custom_led);
}

int button_init(void)
{
	if (!gpio_is_ready_dt(&custom_button))
	{
		return -1;
	}

	int ret = gpio_pin_configure_dt(&custom_button, GPIO_INT_EDGE_TO_ACTIVE);
	gpio_pin_interrupt_configure_dt(&custom_button, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret < 0)
	{
		return ret;
	}
    gpio_init_callback(&button_cb_data, button_pressed, BIT(custom_button.pin)); 	
	gpio_add_callback(custom_button.port, &button_cb_data);
	gpio_add_callback_dt(&custom_button, &button_cb_data);
	return 0;
}