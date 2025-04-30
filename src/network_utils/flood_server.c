// Includes
#include "adapter.h"
#include "serial.h"

// C Standard Library
#include <errno.h>
#include <stdio.h>
#include <string.h>

int main (int argc, char** argv)
{
	if (argc != 3)
	{
		fprintf (stderr, "Invalid arguments. Usage: 'flood-sever <addr> <serial port>'.\n");
		return -1;
	}

	// Copy the address to a buffer (unused characters must be 0'ed)
	char serverAddr [ADDRESS_SIZE] = {};
	strcpy (serverAddr, argv [1]);

	// Open the serial port
	const char* serialPath = argv [2];
	void* serial = serialInit (serialPath);
	if (serial == NULL)
	{
		int code = errno;
		fprintf (stderr, "Failed to open serial port '%s': %s.\n", serialPath, strerror (code));
		return code;
	}

	// Set the device address
	setAddress (serial, serverAddr);

	while (1)
	{
		// Wait for a ping
		char data [DATAGRAM_SIZE];
		char clientAddr [ADDRESS_SIZE];
		uint16_t size = receive (serial, data, clientAddr, -1);

		// Check the datagram was a ping
		if (strcmp (data, "flood_request__"))
			continue;

		printf ("Flood request received from: %s\n", clientAddr);

		// Deliver a response
		strcpy (data, "flood_response_");
		transmit (serial, data, 1024, clientAddr);
	}

	// Close the serial port
	serialClose (serial);
}