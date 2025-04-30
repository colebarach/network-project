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
		fprintf (stderr, "Invalid arguments. Usage: 'wireshark <ASCII/Hex> <serial port>'.\n");
		return -1;
	}

	// Open the serial port
	const char* serialPath = argv [2];
	void* serial = serialInit (serialPath);
	if (serial == NULL)
	{
		int code = errno;
		fprintf (stderr, "Failed to open serial port '%s': %s.\n", serialPath, strerror (code));
		return code;
	}

	bool asciiMode;

	switch (argv [1][0])
	{
	case 'a': // ASCII
		asciiMode = true;
		break;
	case 'h': // Hex
		asciiMode = false;
		break;
	default:
		fprintf (stderr, "Invalid ASCII/Hex option: Acceptable 'h' or 'a'.\n");
		return -1;
	}

	// Set the device address (all 0's for wildcard)
	char addr [ADDRESS_SIZE + 1] = {};
	char data [DATAGRAM_SIZE + 1];
	setAddress (serial, addr);

	while (true)
	{
		// Receive the datagram
		size_t size = receive (serial, data, addr, -1);
		data [DATAGRAM_SIZE] = '\0';

		if (asciiMode)
		{
			// Print the address followed by the payload
			printf ("%s : %s\n", addr, data);
		}
		else
		{
			// Print the address followed by the payload
			for (size_t index = 0; index < ADDRESS_SIZE; ++index)
				printf ("[%02X]", (uint8_t) addr [index]);

			printf (" : ");

			for (size_t index = 0; index < size; ++index)
				printf ("[%02X]", (uint8_t) data [index]);

			printf ("\n");
		}

	}

	// Close the serial port
	serialClose (serial);
}