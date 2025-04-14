// TODO(Barach): This is using an old version of the Pico SDK, should update to a stable release.

// Includes -------------------------------------------------------------------------------------------------------------------

// RPI Pico SDK
#include "pico/stdlib.h"

// C Standard Library
#include <stdlib.h>
#include <string.h>

// Constants ------------------------------------------------------------------------------------------------------------------

#define SEED				1
#define TX_MODE				0

#define BAUDRATE			115200

#define FRAME_TIME_US		(11 * 1000000 / BAUDRATE + 1)
#define INTERFRAME_TIME_US	(FRAME_TIME_US / 8)

#define UART0_TX			0
#define UART0_RX			1
#define UART0_RX_SENSE		2
#define COLLISION_PIN		3

// Functions Prototypes -------------------------------------------------------------------------------------------------------

void waitForIdle ();

void transmitJamFrame ();

bool transmitDatagram (uint8_t* txBuffer, uint16_t txSize);

bool receiveDatagram (uint8_t* rxBuffer, uint16_t rxSize);

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

	#if TX_MODE

	srand (SEED);

	while (true)
	{
		// Transmit the next datagram.
		if (transmitDatagram (txBuffer, sizeof (txBuffer)))
		{
			// If successful, generate a new random payload.
			for (uint8_t index = 0; index < sizeof (txBuffer); ++index)
				txBuffer [index] = rand ();
		}
	}

	#else

	while (true)
	{
		volatile bool test = receiveDatagram (txBuffer, sizeof (txBuffer));
		test = test;
		test = transmitDatagram (txBuffer, sizeof (txBuffer));
		test = test;
	}

	#endif
}

// Core 1 Entrypoint ----------------------------------------------------------------------------------------------------------

int main1 ()
{
	while (true) {}
}

// Functions ------------------------------------------------------------------------------------------------------------------

void waitForIdle ()
{
	// Random CSMA delay, min of 2 frames, max of 10 frames
	uint16_t delayUs = (rand () % (8 * FRAME_TIME_US)) + 2 * FRAME_TIME_US;

	// Block until the bus is idle for the target interval.
	uint16_t count = 0;
	while (true)
	{
		// If not idle, reset timer
		if (!gpio_get (UART0_RX_SENSE))
			count = 0;
		else
			++count;

		if (count >= delayUs)
			break;

		sleep_us (1);
	}
}

void flushReceiveFifo ()
{
	uint8_t buffer;
	while (uart_is_readable(uart0))
		uart_read_blocking (uart0, &buffer, 1);
}

void transmitJamFrame ()
{
	// Debugging output high
	gpio_put (COLLISION_PIN, true);

	// Set the TX pin to a GPIO output.
	gpio_set_function (UART0_TX, GPIO_FUNC_SIO);
	gpio_set_dir (UART0_TX, GPIO_OUT);

	// Hold the bus in the dominant state for 2 frames. This guarantees all devices receive a break error and notice the
	// collision occurred.
	gpio_put (UART0_TX, false);
	sleep_us (2 * FRAME_TIME_US);

	// Set the TX pin back to a UART pin.
	gpio_set_function (UART0_TX, GPIO_FUNC_UART);

	// Debugging output low
	gpio_put (COLLISION_PIN, false);
}

bool transmitDatagram (uint8_t* txBuffer, uint16_t txSize)
{
	// Wait until the bus is idle
	waitForIdle ();

	// Transmit byte by byte, checking for collision.
	for (uint8_t index = 0; index < txSize; ++index)
	{
		uart_get_hw (uart0)->rsr = 0x00;

		// Empty the UART's RX FIFO
		flushReceiveFifo ();

		// Transmit the byte, block until completion
		uart_write_blocking (uart0, txBuffer + index, 1);
		sleep_us (FRAME_TIME_US);

		// No byte read => collision
		if (!uart_is_readable (uart0))
		{
			transmitJamFrame ();
			return false;
		}

		// Read the byte that was transmitted. If break error, parity error, or framing error then a collision occurred.
		uint32_t dr = uart_get_hw (uart0)->dr;
		if ((dr & 0xFF00) != 0x0000 && (dr & 0xFF00) != 0x0800)
		{
			transmitJamFrame ();
			return false;
		}

		// Byte mismatch => collision
		if (txBuffer [index] != (uint8_t) dr)
		{
			transmitJamFrame ();
			return false;
		}
	}

	// Success
	return true;
}

bool receiveDatagram (uint8_t* rxBuffer, uint16_t rxSize)
{
	// Debugging pin
	gpio_put (COLLISION_PIN, false);

	// Block until the first new byte is received.
	flushReceiveFifo ();
	while (!uart_is_readable (uart0));

	uint16_t index = 0;
	while (true)
	{
		// If the datagram exceeds the buffer size, fail.
		if (index >= rxSize)
			return false;

		// If any UART errors occurred, a collision occurred.
		uint32_t dr = uart_get_hw (uart0)->dr;
		if (dr > 0xFF)
		{
			gpio_put (COLLISION_PIN, true);
			return false;
		}

		// Read the byte into the buffer
		rxBuffer [index] = dr;
		++index;

		// Wait for the next byte, or timeout
		uint16_t count = 0;
		while (true)
		{
			if (uart_is_readable (uart0))
				break;

			if (count > INTERFRAME_TIME_US)
				return true;

			++count;

			sleep_us (1);
		}
	}
}