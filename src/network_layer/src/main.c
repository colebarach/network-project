// TODO(Barach): This is using an old version of the Pico SDK, should update to a stable release.

// Includes -------------------------------------------------------------------------------------------------------------------

// RPI Pico SDK
#include "pico/stdlib.h"

// C Standard Library
#include <stdlib.h>
#include <string.h>

// Constants ------------------------------------------------------------------------------------------------------------------

#define ADDR_A 1
#define TX_MODE 0

#define BAUDRATE		115200
#define UART0_TX		0
#define UART0_RX		1
#define UART0_RX_SENSE	2
#define COLLISION_PIN	3

void jamSequence ()
{
	gpio_set_function (UART0_TX, GPIO_FUNC_SIO);
	gpio_set_dir (UART0_TX, GPIO_OUT);

	bool pin = true;
	for (uint8_t i = 0; i < 8; ++i)
	{
		gpio_put (UART0_TX, pin);
		pin = !pin;
		sleep_us (4);
	}

	sleep_us (400);

	gpio_set_function (UART0_TX, GPIO_FUNC_UART);
}

// Core 0 Entrypoint ----------------------------------------------------------------------------------------------------------

int main ()
{
	uart_init (uart0, BAUDRATE);

	gpio_set_function (UART0_TX, GPIO_FUNC_UART);
	gpio_set_function (UART0_RX, GPIO_FUNC_UART);

	gpio_init (UART0_RX_SENSE);
	gpio_set_dir (UART0_RX_SENSE, GPIO_IN);
	gpio_init (COLLISION_PIN);
	gpio_set_dir (COLLISION_PIN, GPIO_OUT);

	#if ADDR_A
	uint8_t txBuffer [4] = { 0xAB, 0xCD, 0x12, 0x34 };
	#else
	uint8_t txBuffer [4] = { 0x87, 0x65, 0xDC, 0xBA };
	#endif

	uint8_t rxBuffer;

	#if TX_MODE

	while (true)
	{
		// Random CSMA delay, min of 400 us
		uint16_t target = (rand () % 512) + 400;

		// Block until the bus is idle for the target interval.
		uint16_t count = 0;
		while (true)
		{
			// If not idle, reset timer
			if (!gpio_get (UART0_RX_SENSE))
				count = 0;
			else
				++count;

			if (count > target)
				break;

			sleep_us (1);
		}

		gpio_put (COLLISION_PIN, false);

		// Transmit byte by byte, checking for collision.
		for (uint8_t index = 0; index < sizeof (txBuffer); ++index)
		{
			// Empty the UART's RX FIFO
			while (uart_is_readable(uart0))
				uart_read_blocking (uart0, &rxBuffer, sizeof (rxBuffer));

			// Transmit the byte
			uart_write_blocking (uart0, txBuffer + index, 1);

			sleep_us (96);

			// No byte read => collision
			if (!uart_is_readable (uart0))
			{
				gpio_put (COLLISION_PIN, true);
				jamSequence ();
				break;
			}

			// Read the byte that was transmitted
			uart_read_blocking (uart0, &rxBuffer, sizeof (rxBuffer));

			// Byte mismatch => collision
			if (txBuffer [index] != rxBuffer)
			{
				gpio_put (COLLISION_PIN, true);
				jamSequence ();
				break;
			}
		}
	}

	#else

	while (true)
	{
		// Read the byte that was transmitted
		while (!uart_is_readable (uart0))
			tight_loop_contents ();

		gpio_put (COLLISION_PIN, false);

		volatile uint32_t d = uart_get_hw (uart0)->dr;
		if (d > 0xFF)
		{
			gpio_put (COLLISION_PIN, true);

			sleep_us (100);

			while (uart_is_readable (uart0))
				uart_read_blocking (uart0, &rxBuffer, sizeof (rxBuffer));
		}
	}

	#endif
}

// Core 1 Entrypoint ----------------------------------------------------------------------------------------------------------

int main1 ()
{
	while (true) {}
}
