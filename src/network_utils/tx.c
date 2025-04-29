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
		fprintf (stderr, "Invalid arguments. Usage: 'tx <src addr> <dest addr> <serial port>'.\n");
		return -1;
	}

	// Copy the address to a buffer (unused characters must be 0'ed)
	char srcAddr [ADDRESS_SIZE] = {};
	strcpy (srcAddr, argv [1]);

	// Copy the address to a buffer (unused characters must be 0'ed)
	char destAddr [ADDRESS_SIZE] = {};
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
	setAddress (serial, srcAddr);

	// Read the data
	char data [DATAGRAM_SIZE + 1];
	fprintf (stderr, "Message: ");
	fscanf (stdin, "%[^\n]s", data);
	getc (stdin);
	data [DATAGRAM_SIZE] = '\0';

	// Transmit the datagram
	transmit (serial, data, strlen (data), destAddr);

	// Close the serial port
	fclose (serial);
}