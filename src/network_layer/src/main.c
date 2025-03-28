// TODO(Barach): This is using an old version of the Pico SDK, should update to a stable release.

// Includes -------------------------------------------------------------------------------------------------------------------

// RPI Pico SDK
#include "pico/stdlib.h"

// C Standard Library
#include <stdlib.h>
#include <string.h>

// Constants ------------------------------------------------------------------------------------------------------------------

#define SEED 0
#define TX_MODE 1

#define BAUDRATE		115200
#define UART0_TX		0
#define UART0_RX		1
#define UART0_RX_SENSE	2
#define COLLISION_PIN	3

void jamSequence ()
{
	gpio_set_function (UART0_TX, GPIO_FUNC_SIO);
	gpio_set_dir (UART0_TX, GPIO_OUT);

	gpio_put (UART0_TX, false);
	sleep_us (120);

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

	uint8_t txBuffer [4];

	uint8_t rxBuffer;

	#if TX_MODE

	srand (SEED);

	while (true)
	{
		// Random payload
		for (uint8_t index = 0; index < sizeof (txBuffer); ++index)
			txBuffer [index] = rand ();

		// Random CSMA delay, min of 400 us
		uint16_t target = (rand () % 512) + 120;

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
			uart_get_hw (uart0)->rsr = 0x00;

			// Empty the UART's RX FIFO
			while (uart_is_readable(uart0))
				uart_read_blocking (uart0, &rxBuffer, sizeof (rxBuffer));

			// Transmit the byte
			uart_write_blocking (uart0, txBuffer + index, 1);

			sleep_us (100);

			// No byte read => collision
			if (!uart_is_readable (uart0))
			{
				gpio_put (COLLISION_PIN, true);
				jamSequence ();
				break;
			}

			// Read the byte that was transmitted. UART error => collision
			uint32_t dr = uart_get_hw (uart0)->dr;
			rxBuffer = (uint8_t) dr;
			if ((dr & 0xFF00) != 0x0000 && (dr & 0xFF00) != 0x0800)
			{
				gpio_put (COLLISION_PIN, true);
				jamSequence ();
				break;
			}

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

		uint32_t dr = uart_get_hw (uart0)->dr;
		if (dr > 0xFF)
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
