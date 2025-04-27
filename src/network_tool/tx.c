// C Standard Library
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define ADDRESS_SIZE 8
#define DATAGRAM_SIZE 1024

int main (int argc, char** argv)
{
	if (argc != 2)
	{
		fprintf (stderr, "Invalid arguments. Usage: 'tx <serial port name>'.\n");
		return -1;
	}

	const char* serialPath = argv [1];

	uint8_t address [ADDRESS_SIZE + 1];
	char data [DATAGRAM_SIZE + 1];

	FILE* fSerial = fopen (serialPath, "w");
	if (fSerial == NULL)
	{
		int code = errno;
		fprintf (stderr, "Failed to open serial port '%s': %s.\n", serialPath, strerror (code));
		return code;
	}

	fprintf (stderr, "To: ");
	fscanf (stdin, "%[^\n]s", address);
	getc (stdin);
	address [ADDRESS_SIZE] = '\0';

	fprintf (stderr, "Message: ");
	fscanf (stdin, "%[^\n]s", data);
	getc (stdin);
	data [DATAGRAM_SIZE] = '\0';

	uint8_t size = (strlen (data) + 1 + 3) / 4 - 1;

	fputc (0x7C, fSerial);

	for (uint8_t index = 0; index < ADDRESS_SIZE; ++index)
		fputc (address [index], fSerial);

	fputc (size, fSerial);

	for (uint8_t index = 0; index < (size + 1) * 4; ++index)
		fputc (data [index], fSerial);
}