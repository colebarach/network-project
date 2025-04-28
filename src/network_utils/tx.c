// C Standard Library
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define ADDRESS_SIZE 8
#define DATAGRAM_SIZE 1024

void setAddr (FILE* serial, const char* addr)
{
	fputc (0x7E, serial);

	for (uint8_t index = 0; index < ADDRESS_SIZE; ++index)
		fputc (addr [index], serial);
}

void transmit (FILE* serial, const char* data, uint16_t dataCount, const char* addr)
{
	uint8_t size = (strlen (data) + 1 + 3) / 4 - 1;

	fputc (0x7C, serial);

	for (uint8_t index = 0; index < ADDRESS_SIZE; ++index)
		fputc (addr [index], serial);

	fputc (size, serial);

	for (uint8_t index = 0; index < (size + 1) * 4; ++index)
		fputc (data [index], serial);
}

int main (int argc, char** argv)
{
	if (argc != 4)
	{
		fprintf (stderr, "Invalid arguments. Usage: 'tx <src addr> <dest addr> <serial port>'.\n");
		return -1;
	}

	char srcAddr [ADDRESS_SIZE] = {};
	strcpy (srcAddr, argv [1]);

	char destAddr [ADDRESS_SIZE] = {};
	strcpy (destAddr, argv [2]);

	const char* serialPath = argv [3];

	char data [DATAGRAM_SIZE + 1];

	FILE* fSerial = fopen (serialPath, "w");
	if (fSerial == NULL)
	{
		int code = errno;
		fprintf (stderr, "Failed to open serial port '%s': %s.\n", serialPath, strerror (code));
		return code;
	}

	fprintf (stderr, "Message: ");
	fscanf (stdin, "%[^\n]s", data);
	getc (stdin);
	data [DATAGRAM_SIZE] = '\0';

	setAddr (fSerial, srcAddr);
	transmit (fSerial, data, strlen (data), destAddr);
}