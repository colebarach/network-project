// Includes
#include "adapter.h"
#include "serial.h"

// POSIX
#include <pthread.h>

// C Standard Library
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

size_t count = 0;

void* serial = NULL;
char serverAddr [ADDRESS_SIZE] = {};

void* receiveThread (void* arg)
{
	char payload [DATAGRAM_SIZE];

	// Wait for the response
	while (true)
	{
		char addr [ADDRESS_SIZE];
		if (!receive (serial, payload, addr, 2))
			return (void*) -1;

		if (strcmp (addr, serverAddr) == 0 && strcmp (payload, "flood_response_") == 0)
			++count;

		printf ("Flood response received.\n");

		if (count >= 64)
			return (void*) 0;
	}
}

int main (int argc, char** argv)
{
	if (argc != 4)
	{
		fprintf (stderr, "Invalid arguments. Usage: 'flood-client <client addr> <server addr> <serial port>'.\n");
		return -1;
	}

	// Copy the address to a buffer (unused characters must be 0'ed)
	char clientAddr [ADDRESS_SIZE] = {};
	strcpy (clientAddr, argv [1]);

	// Copy the address to a buffer (unused characters must be 0'ed)
	strcpy (serverAddr, argv [2]);

	// Open the serial port
	const char* serialPath = argv [3];
	serial = serialInit (serialPath);
	if (serial == NULL)
	{
		int code = errno;
		fprintf (stderr, "Failed to open serial port '%s': %s.\n", serialPath, strerror (code));
		return code;
	}

	// Set the device address
	setAddress (serial, clientAddr);

	struct timespec timeStart, timeEnd;

	timespec_get(&timeStart, TIME_UTC);

	pthread_t rxThread;
	if (pthread_create (&rxThread, NULL, receiveThread, NULL) != 0)
	{
		int code = errno;
		printf ("Failed to create RX thread: %s", strerror (code));
		return code;
	}

	while (count < 64)
	{
		char data [252] = "flood_request__";
		transmit (serial, data, sizeof (data), serverAddr);

		struct timespec sleepTime = { .tv_sec = 0, .tv_nsec = 500000 };
		struct timespec remaining;
		nanosleep (&sleepTime, &remaining);
	}

	printf ("Flood end.\n");

	void* ret;
	pthread_join (rxThread, &ret);
	if (((long int) ret) == -1)
	{
		printf ("Flood response timed out.\n");
		return -1;
	}

	timespec_get(&timeEnd, TIME_UTC);

	float timeDiff = timeEnd.tv_sec - timeStart.tv_sec + (timeEnd.tv_nsec - timeStart.tv_nsec) / 1000000000.0f;
	printf ("Time ellapsed: %f ms.\n", timeDiff * 1000.0f);

	// Close the serial port
	serialClose (serial);
}