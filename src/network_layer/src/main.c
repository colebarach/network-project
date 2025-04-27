// TODO(Barach): This is using an old version of the Pico SDK, should update to a stable release.

// Includes -------------------------------------------------------------------------------------------------------------------

// RPI Pico SDK
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/sync.h"

// C Standard Library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Constants ------------------------------------------------------------------------------------------------------------------

// Datagram Packaging
#define DATAGRAM_SIZE		1024
#define ADDRESS_SIZE		8

// Data-link Timing
#define BAUDRATE			115200 // TODO(Barach): Increase baudrate
#define FRAME_TIME_US		(11 * 1000000 / BAUDRATE + 1)
#define INTERFRAME_TIME_US	(FRAME_TIME_US / 2)

// Inputs / Outputs
#define UART0_TX			0
#define UART0_RX			1
#define UART0_RX_SENSE		2
#define COLLISION_PIN		3

// Globals --------------------------------------------------------------------------------------------------------------------

// The datagram to transmit (from the USB thread, to UART thread).
uint8_t txBuffer [DATAGRAM_SIZE];

// The datagram that was received (from the UART thread, to the USB thread).
uint8_t rxBuffer [DATAGRAM_SIZE];

// Mutex guarding access to the UART peripheral. This is used to prevent multiple threads reading / writing at the same time.
mutex_t uartMutex;

// Functions Prototypes -------------------------------------------------------------------------------------------------------

void waitForIdle ();

void waitForDatagram ();

void flushReceiveFifo ();

void transmitJamFrame ();

bool transmitDatagram (uint8_t* txBuffer, uint16_t txSize);

bool receiveDatagram (uint8_t* data, uint16_t* dataCount);

void main1 ();

// Core 0 Entrypoint ----------------------------------------------------------------------------------------------------------

int main ()
{
	// Initialize the USB interface
	stdio_init_all ();

	// Initialize the UART peripheral
	uart_init (uart0, BAUDRATE);
	gpio_set_function (UART0_TX, GPIO_FUNC_UART);
	gpio_set_function (UART0_RX, GPIO_FUNC_UART);

	// Initialize the GPIO
	gpio_init (UART0_RX_SENSE);
	gpio_set_dir (UART0_RX_SENSE, GPIO_IN);
	gpio_init (COLLISION_PIN);
	gpio_set_dir (COLLISION_PIN, GPIO_OUT);

	// Initialize the random number generator
	srand (0);

	// Initialize the mutices
	mutex_init (&uartMutex);

	// Start UART thread
	multicore_reset_core1 ();
	multicore_launch_core1 (&main1);

	// Transmit Thread
	while (true)
	{
		// Command type (1 byte)
		if (fgetc (stdin) != 0x7C)
			continue;

		// Address (8 byte)
		uint8_t addr [ADDRESS_SIZE];
		for (uint16_t index = 0; index < ADDRESS_SIZE; ++index)
			txBuffer [index] = fgetc (stdin);

		// Size (1 byte)
		uint16_t size = (fgetc (stdin) + 1) * 4;

		// Data
		for (uint16_t index = ADDRESS_SIZE; index < size + ADDRESS_SIZE; ++index)
			txBuffer [index] = fgetc (stdin);

		mutex_enter_blocking (&uartMutex);
		transmitDatagram (txBuffer, ADDRESS_SIZE + size);
		mutex_exit (&uartMutex);
	}
}

// Core 1 Entrypoint ----------------------------------------------------------------------------------------------------------

void main1 ()
{
	// Receive Thread
	while (true)
	{
		// Wait until a datagram is received
		uint16_t dataCount;
		waitForDatagram ();
		mutex_enter_blocking (&uartMutex);
		if (!receiveDatagram (rxBuffer, &dataCount))
		{
			mutex_exit (&uartMutex);
			continue;
		}
		mutex_exit (&uartMutex);

		// Send receive response type
		printf ("%c", 0x7D);

		// Send the address
		for (uint16_t index = 0; index < ADDRESS_SIZE; ++index)
			printf ("%c", rxBuffer [index]);

		// Send the payload size
		uint8_t size = (dataCount - ADDRESS_SIZE) / 4 - 1;
		printf ("%c", size);

		// Send the payload
		for (uint16_t index = 0; index < (size + 1) * 4; ++index)
			printf ("%c", rxBuffer [ADDRESS_SIZE + index]);
	}
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

void waitForDatagram ()
{
	// Block until the first new byte is received.
	flushReceiveFifo ();
	while (!uart_is_readable (uart0));
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
		// Clear the error register
		uart_get_hw (uart0)->rsr = 0x00;

		// Empty the UART's RX FIFO
		flushReceiveFifo ();

		// Transmit the byte, block until completion
		uart_write_blocking (uart0, txBuffer + index, 1);
		sleep_us (FRAME_TIME_US + 10); // TODO(Barach): Does this work? Can it be lowered?

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

bool receiveDatagram (uint8_t* data, uint16_t* dataCount)
{
	// Debugging pin
	gpio_put (COLLISION_PIN, false);

	// If no data is available, return.
	if (!uart_is_readable (uart0))
		return false;

	// Read the datagram into the buffer, byte by byte.
	*dataCount = 0;
	while (true)
	{
		// If the datagram exceeds the buffer size, fail.
		if (*dataCount >= DATAGRAM_SIZE + ADDRESS_SIZE)
			return false;

		// If any UART errors occurred, a collision occurred.
		uint32_t dr = uart_get_hw (uart0)->dr;
		if (dr > 0xFF)
		{
			gpio_put (COLLISION_PIN, true);
			return false;
		}

		// Read the byte into the buffer
		data [*dataCount] = dr;
		++(*dataCount);

		// Wait for the next byte, or timeout
		uint16_t timeout = 0;
		while (true)
		{
			if (uart_is_readable (uart0))
				break;

			if (timeout > FRAME_TIME_US + INTERFRAME_TIME_US) // TODO(Barach): Does this work?
			{
				// TODO(Barach): This needs redone.
				// Reject malformed datagrams. Require payload size be be a non-zero multiple of 4.
				// if (*dataCount < ADDRESS_SIZE + 4 || *dataCount % 4 != 0)
				//	return false;

				return true;
			}

			++timeout;
			sleep_us (1);
		}
	}
}