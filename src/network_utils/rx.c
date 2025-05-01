// Includes
#include "adapter.h"
#include "serial.h"

// C Standard Library
#include <errno.h>
#include <stdio.h>
#include <string.h>

int main (int argc, char** argv)
{
	if (argc != 4)
	{
		fprintf (stderr, "Invalid arguments. Usage: 'rx <src addr> <dest addr> <serial port>'.\n");
		return -1;
	}

	// Copy the address to a buffer (unused characters must be 0'ed)
	char srcAddr [ADDRESS_SIZE + 1] = {};
	strcpy (srcAddr, argv [1]);

	// Copy the address to a buffer (unused characters must be 0'ed)
	char destAddr [ADDRESS_SIZE + 1] = {};
	strcpy (destAddr, argv [2]);

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
	setAddress (serial, destAddr);

	while (true)
	{
		// Receive the datagram
		char addr [ADDRESS_SIZE + 1];
		char data [DATAGRAM_SIZE + 1];
		receive (serial, data, addr, -1);
		data [DATAGRAM_SIZE] = '\0';

		// Ignore datagrams not from the specific source.
		if (strcmp (srcAddr, addr) != 0)
			continue;

		// Print the datagram
		printf ("%s\n", data);
		fflush (stdout);
	}

	// Close the serial port
	serialClose (serial);
}