// Header
#include "adapter.h"

// Includes
#include "serial.h"

// C Standard Library
#include <stdbool.h>
#include <time.h>

void setAddress (void* serial, const void* addr)
{
	// Write the command type, followed by the address to set.
	uint8_t type = 0x7E;
	serialWrite (serial, &type, sizeof (type));
	serialWrite (serial, addr, ADDRESS_SIZE);
}

void transmit (void* serial, const void* payload, uint16_t size, const void* destAddr)
{
	// Write the command type (1 byte).
	uint8_t type = 0x7C;
	serialWrite (serial, &type, sizeof (type));

	// Write the destination address (8 bytes).
	serialWrite (serial, destAddr, ADDRESS_SIZE);

	// Convert and write the payload size (1 byte).
	uint8_t convertedSize = (size + 3) / 4 - 1;
	serialWrite (serial, &convertedSize, sizeof (size));

	// Write the payload. Note that this uses the converted size, not the user-provided size.
	serialWrite (serial, payload, (size + 1) * 4);
}

size_t receive (void* serial, void* payload, char* srcAddr, time_t timeout)
{
	time_t deadline = time (NULL) + timeout;

	// Wait until the receive response starts
	uint8_t type;
	while (1)
	{
		// If the type byte was read, break.
		if (serialRead (serial, &type, sizeof (type)) != 0 && type == 0x7D)
			break;

		// Check for timeout.
		if (timeout != -1 && time (NULL) > deadline)
			return 0;
	}

	// Read the source address.
	if (serialRead (serial, srcAddr, ADDRESS_SIZE) != ADDRESS_SIZE)
		return 0;

	// Read the payload size.
	uint8_t unconvertedSize;
	if (serialRead (serial, &unconvertedSize, sizeof (unconvertedSize)) != sizeof (unconvertedSize))
		return 0;
	uint16_t size = (unconvertedSize + 1) * 4;

	// Read the payload
	if (serialRead (serial, payload, size) != size)
		return 0;
	return size;
}