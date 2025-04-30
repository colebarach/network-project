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

size_t targetCount;
size_t receiveCount = 0;

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
			++receiveCount;

		if (receiveCount >= targetCount)
			return (void*) 0;
	}
}

int main (int argc, char** argv)
{
	if (argc != 5)
	{
		fprintf (stderr, "Invalid arguments. Usage: 'flood-client <client addr> <server addr> <packet count> <serial port>'.\n");
		return -1;
	}

	// Copy the address to a buffer (unused characters must be 0'ed)
	char clientAddr [ADDRESS_SIZE] = {};
	strcpy (clientAddr, argv [1]);

	// Copy the address to a buffer (unused characters must be 0'ed)
	strcpy (serverAddr, argv [2]);

	// Get the number of packets to use.
	targetCount = atoi (argv [3]);

	// Open the serial port
	const char* serialPath = argv [4];
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

	size_t sentCount = 0;
	while (receiveCount < targetCount)
	{
		char data [1024] = "flood_request__";
		transmit (serial, data, sizeof (data), serverAddr);
		++sentCount;

		struct timespec sleepTime = { .tv_sec = 0, .tv_nsec = 35000000 };
		struct timespec remaining;
		nanosleep (&sleepTime, &remaining);
	}

	void* ret;
	pthread_join (rxThread, &ret);
	if (((long int) ret) == -1)
	{
		printf ("Flood response timed out.\n");
		return -1;
	}

	timespec_get(&timeEnd, TIME_UTC);

	float timeDiff = timeEnd.tv_sec - timeStart.tv_sec + (timeEnd.tv_nsec - timeStart.tv_nsec) / 1000000000.0f;
	size_t sentKibibytes = sentCount * 8;
	size_t receiveKibibytes = receiveCount * 8;
	printf ("Sent: %lu packets (%lu KiB).\nReceived: %lu packets (%lu KiB).\nTime ellapsed: %f ms.\n",
		sentCount, sentKibibytes, receiveCount, receiveKibibytes, timeDiff * 1000.0f);
	printf ("Effective throughput: %f kbps\n", receiveCount * 1024.0f * 8.0f * 2.0f / timeDiff / 1000.0f);

	// Close the serial port
	serialClose (serial);
}