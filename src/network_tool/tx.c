// C Standard Library
#include <stdio.h>
#include <stdint.h>
#include <string.h>

int main (int argc, char** argv)
{
	uint8_t address [8];
	char data [1024];

	fprintf (stderr, "Address: ");
	fscanf (stdin, "%s", address);

	fprintf (stderr, "Data:");
	fscanf (stdin, "%s", data);
	uint8_t size = strlen (data);

	fputc (0x7C, stdout);

	for (uint8_t index = 0; index < sizeof (address); ++index)
		fputc (address [index], stdout);

	fputc (size / 4, stdout);

	for (uint8_t index = 0; index < size; ++index)
		fputc (data [index], stdout);
}