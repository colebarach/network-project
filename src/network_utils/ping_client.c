// Includes
#include "adapter.h"
#include "serial.h"

// C Standard Library
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

int main (int argc, char** argv)
{
	if (argc != 4)
	{
		fprintf (stderr, "Invalid arguments. Usage: 'ping-client <client addr> <server addr> <serial port>'.\n");
		return -1;
	}

	// Copy the address to a buffer (unused characters must be 0'ed)
	char clientAddr [ADDRESS_SIZE] = {};
	strcpy (clientAddr, argv [1]);

	// Copy the address to a buffer (unused characters must be 0'ed)
	char serverAddr [ADDRESS_SIZE] = {};
	strcpy (serverAddr, argv [2]);

	// Open the serial port
	const char* serialPath = argv [3];
	void* serial = serialInit (serialPath);
	if (serial == NULL)
	{
		int code = errno;
		fprintf (stderr, "Failed to open serial port '%s': %s.\n", serialPath, strerror (code));
		return code;
	}

	// Set the device address
	setAddress (serial, clientAddr);

	struct timespec timeTx, timeRx;

	for (uint8_t index = 0; index < 8; ++index)
	{
		// Send a ping
		timespec_get(&timeTx, TIME_UTC);
		char data [] = "ping_request___";
		transmit (serial, data, sizeof (data), serverAddr);

		// Wait for the response
		while (1)
		{
			char addr [ADDRESS_SIZE];
			receive (serial, data, addr, -1);
			if (strcmp (addr, serverAddr) != 0 || strcmp (data, "ping_response__") != 0)
				continue;

			timespec_get(&timeRx, TIME_UTC);
			break;
		}

		float time = (timeRx.tv_sec - timeTx.tv_sec) + (timeRx.tv_nsec - timeTx.tv_nsec) / 1000000000.0f;
		printf ("Ping received: Latency of %f ms\n", time * 1000);
	}

	// Close the serial port
	serialClose (serial);
}