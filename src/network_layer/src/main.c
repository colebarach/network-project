// Includes -------------------------------------------------------------------------------------------------------------------

// RPI Pico SDK
#include "pico/stdlib.h"

// Core 0 Entrypoint ----------------------------------------------------------------------------------------------------------

int main ()
{
	const uint16_t LED_PIN = 0;
	gpio_init (LED_PIN);
	gpio_set_dir (LED_PIN, GPIO_OUT);

	while (true)
	{
		sio_hw->gpio_out = 0x01;
		sleep_ms (250);
		sio_hw->gpio_out = 0x00;
		sleep_ms (250);
	}
}

// Core 1 Entrypoint ----------------------------------------------------------------------------------------------------------

int main1 ()
{
	while (true) {}
}
