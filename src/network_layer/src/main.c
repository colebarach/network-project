// TODO(Barach): This is using an old version of the Pico SDK, should update to a stable release.

// Includes -------------------------------------------------------------------------------------------------------------------

// RPI Pico SDK
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/sync.h"

// POSIX Library
#include <unistd.h>

// C Standard Library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Constants ------------------------------------------------------------------------------------------------------------------

// Datagram Packaging
#define DATAGRAM_SIZE		1024
#define ADDRESS_SIZE		8

// Data-link Timing
#define BAUDRATE			921600
#define FRAME_TIME_US		(11 * 1000000 / BAUDRATE + 1)
#define INTERFRAME_TIME_US	(FRAME_TIME_US / 4)

// Inputs / Outputs
#define UART0_TX			0
#define UART0_RX			1
#define UART0_RX_SENSE		2
#define COLLISION_PIN		3

// Globals --------------------------------------------------------------------------------------------------------------------

// The device's address. Received datagrams not matching this address are ignored.
uint8_t address [ADDRESS_SIZE];

// Mutex guarding access to the UART peripheral. This is used to prevent multiple threads reading / writing at the same time.
mutex_t uartMutex;

// Functions Prototypes -------------------------------------------------------------------------------------------------------

void waitForIdle ();

void waitForDatagram ();

void flushReceiveFifo ();

void transmitJamFrame ();

bool transmitDatagram (uint8_t* destAddr, uint8_t* payload, uint16_t payloadSize);

bool receiveDatagram (uint8_t* srcAddr, uint8_t* payload, uint16_t* payloadSize);

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
	uint8_t payload [DATAGRAM_SIZE];
	while (true)
	{
		// Command type (1 byte)
		uint8_t type = fgetc (stdin);
		if (type == 0x7C)
		{
			// Transmit command

			// Destination address (8 byte)
			uint8_t destAddr [ADDRESS_SIZE];
			for (uint16_t index = 0; index < ADDRESS_SIZE; ++index)
				destAddr [index] = fgetc (stdin);

			// Size (1 byte)
			uint16_t payloadSize = ((uint16_t) fgetc (stdin) + 1) * 4;

			// Payload
			for (uint16_t index = 0; index < payloadSize; ++index)
				payload [index] = fgetc (stdin);

			mutex_enter_blocking (&uartMutex);
			transmitDatagram (destAddr, payload, payloadSize);
			mutex_exit (&uartMutex);
		}
		else if (type == 0x7E)
		{
			// Address (8 byte)
			for (uint16_t index = 0; index < ADDRESS_SIZE; ++index)
				address [index] = fgetc (stdin);
		}
	}
}

// Core 1 Entrypoint ----------------------------------------------------------------------------------------------------------

void main1 ()
{
	// Receive Thread
	uint8_t payload [DATAGRAM_SIZE];
	uint8_t srcAddr [ADDRESS_SIZE];
	uint16_t payloadSize;
	while (true)
	{
		// Wait until a datagram is received
		waitForDatagram ();
		mutex_enter_blocking (&uartMutex);
		if (!receiveDatagram (srcAddr, payload, &payloadSize))
		{
			mutex_exit (&uartMutex);
			continue;
		}
		mutex_exit (&uartMutex);

		// Send receive response type
		putchar_raw (0x7D);

		// Send the source address
		write (1, srcAddr, ADDRESS_SIZE);

		// Send the payload size
		uint8_t size = payloadSize / 4 - 1;
		putchar_raw (size);

		// Send the payload
		write (1, payload, (size + 1) * 4);
	}
}

// Functions ------------------------------------------------------------------------------------------------------------------

void waitForIdle ()
{
	// Random CSMA delay, min of 1 frame, max of 9 frames
	uint16_t delayUs = (rand () % (8 * FRAME_TIME_US)) + FRAME_TIME_US;

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

bool transmitByte (uint8_t data)
{
	// Clear the error register
	uart_get_hw (uart0)->rsr = 0x00;

	// Empty the UART's RX FIFO
	flushReceiveFifo ();

	// Transmit the byte, block until completion
	uart_write_blocking (uart0, &data, sizeof (data));
	sleep_us (FRAME_TIME_US + INTERFRAME_TIME_US / 2);

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
	if (data != (uint8_t) dr)
	{
		transmitJamFrame ();
		return false;
	}

	return true;
}

int receiveByte (uint8_t* data)
{
	// Wait for the next byte, or timeout
	uint16_t timeout = 0;
	while (true)
	{
		if (uart_is_readable (uart0))
			break;

		if (timeout > FRAME_TIME_US + INTERFRAME_TIME_US)
			return -1;

		++timeout;
		sleep_us (1);
	}

	// If any UART errors occurred, a collision occurred.
	uint32_t dr = uart_get_hw (uart0)->dr;
	if (dr > 0xFF)
	{
		gpio_put (COLLISION_PIN, true);
		return -2;
	}

	// Read the byte into the buffer
	if (data != NULL)
		*data = dr;
	return 0;
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

bool transmitDatagram (uint8_t* destAddr, uint8_t* payload, uint16_t payloadSize)
{
	// Wait until the bus is idle
	waitForIdle ();

	// Transmit the source address (this device), checking for collision.
	for (uint8_t index = 0; index < ADDRESS_SIZE; ++index)
		if (!transmitByte (address [index]))
			return false;

	// Transmit the destination address, checking for collision.
	for (uint8_t index = 0; index < ADDRESS_SIZE; ++index)
		if (!transmitByte (destAddr [index]))
			return false;

	// Transmit the payload, checking for collision
	for (uint16_t index = 0; index < payloadSize; ++index)
		if (!transmitByte (payload [index]))
			return false;

	// Success
	return true;
}

bool receiveDatagram (uint8_t* srcAddr, uint8_t* payload, uint16_t* payloadSize)
{
	// Debugging pin
	gpio_put (COLLISION_PIN, false);

	// If no data is available, return.
	if (!uart_is_readable (uart0))
		return false;

	// Read the source address, failing if a timeout or a collision occurs.
	for (uint8_t index = 0; index < ADDRESS_SIZE; ++index)
		if (receiveByte (srcAddr + index) != 0)
			return false;

	// Read the destination address, failing if a timeout or a collision occurs.
	uint8_t destAddr [ADDRESS_SIZE];
	for (uint8_t index = 0; index < ADDRESS_SIZE; ++index)
		if (receiveByte (destAddr + index) != 0)
			return false;

	// Read the payload, byte by byte.
	*payloadSize = 0;
	while (true)
	{
		switch (receiveByte (payload + *payloadSize))
		{
		case 0: // Byte received
			++(*payloadSize);
			break;
		case -1: // Timeout
			// Addresses starting with 0 are wildcards, they should receive all messages.
			if (address [0] == 0)
				return true;

			// Otherwise, only return true if the address matches ours.
			return memcmp (destAddr, address, ADDRESS_SIZE) == 0;
		case -2: // Collision
			return false;
		}
	}
}