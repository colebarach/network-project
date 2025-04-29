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
		fprintf (stderr, "Invalid arguments. Usage: 'rx <addr> <serial port>'.\n");
		return -1;
	}

	// Copy the address to a buffer (unused characters must be 0'ed)
	char destAddr [ADDRESS_SIZE] = {};
	strcpy (destAddr, argv [1]);

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
	setAddress (serial, destAddr);

	// Receive the datagram
	char srcAddr [ADDRESS_SIZE + 1];
	char data [DATAGRAM_SIZE + 1];
	receive (serial, data, srcAddr, -1);
	srcAddr [ADDRESS_SIZE] = '\0';
	data [DATAGRAM_SIZE] = '\0';

	// Print the datagram
	printf ("%s: %s\n", srcAddr, data);

	// Close the serial port
	fclose (serial);
}