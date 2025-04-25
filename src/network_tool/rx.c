// C Standard Library
#include <stdio.h>
#include <stdint.h>

int main (int argc, char** argv)
{
	uint8_t address [8];
	char data [1024];

	while (1)
		if (getc (stdin) == 0x7D)
			break;

	printf ("Addr: ");

	for (uint8_t index = 0; index < 8; ++index)
	{
		address [index] = getc (stdin);
		putc (address [index], stdout);
		// printf ("%02X ", address [index]);
	}

	uint8_t size = getc (stdin);
	printf ("\nSize: %u", size);

	printf ("\nData: ");

	for (uint8_t index = 0; index < size; ++index)
	{
		data [index] = getc (stdin);
		putc (data [index], stdout);
	}

	printf ("\n");
}