// Header
#include "serial.h"

#define DEBUGGING 0

#ifndef _WIN32

// POSIX Implementation -------------------------------------------------------------------------------------------------------

// C Standard Library
#include <stdio.h>

// POSIX
#include <fcntl.h>
#include <poll.h>
#include <termio.h>
#include <unistd.h>

void* serialInit (const char* port)
{
	// Open and configure the port as a TTY.

	int fd = open (port, O_RDWR);
	if (fd < 0)
		return NULL;

	struct termios tty;
	if (tcgetattr (fd, &tty) != 0)
		return NULL;

	// From: https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/

	tty.c_cflag &= ~PARENB;						// Disable parity
	tty.c_cflag &= ~CSTOPB;						// One stop bit
	tty.c_cflag &= ~CSIZE;						// Clear all bits that set the data size
	tty.c_cflag |= CS8;							// 8 bits per byte
	tty.c_cflag &= ~CRTSCTS;					// Disable flow control
	tty.c_cflag |= CREAD | CLOCAL;				// Turn on READ & Ignore control lines

	tty.c_lflag &= ~ICANON;						// Disable canonical mode
	tty.c_lflag &= ~ECHO;						// Disable echo
	tty.c_lflag &= ~ECHOE;						// Disable erasure
	tty.c_lflag &= ~ECHONL;						// Disable new-line echo
	tty.c_lflag &= ~ISIG;						// Disable interpretation of INTR, QUIT and SUSP

	tty.c_iflag &= ~IXON;
	tty.c_iflag &= ~IXOFF;
	tty.c_iflag &= ~IXANY;
	tty.c_iflag |= IGNBRK;
	tty.c_iflag &= ~BRKINT;
	tty.c_iflag &= ~PARMRK;
	tty.c_iflag &= ~ISTRIP;
	tty.c_iflag &= ~INLCR;
	tty.c_iflag |= IGNCR;
	tty.c_iflag &= ~ICRNL;

	tty.c_oflag &= ~OPOST;						// Prevent special interpretation of output bytes (e.g. newline chars)
	tty.c_oflag &= ~ONLCR;						// Prevent conversion of newline to carriage return/line feed
	tty.c_oflag &= ~OCRNL;
	tty.c_oflag &= ~ONLRET;
	tty.c_oflag |= ONOCR;

	tty.c_cc[VTIME] = 0;
	tty.c_cc[VMIN] = 0;
	tty.c_cflag |= CS8;

	if (tcsetattr (fd, TCSANOW, &tty) != 0)
		return NULL;

	// As the serial hander is OS dependent and the return value is OS agnostic, we have to cast our handler into a generic
	// type (void* in this case).
	return (void*) ((long int) fd);
}

void serialClose (void* serial)
{
	close ((long int) serial);
}

void serialWrite (void* serial, const void* data, size_t size)
{
	#if DEBUGGING
	printf ("Write: ");
	for (size_t index = 0; index < size; ++index)
		printf ("%02X ", ((uint8_t*) data) [index]);
	printf ("\n");
	#endif // DEBUGGING

	write ((long int) serial, data, size);
}

bool serialRead (void* serial, void* data, size_t size, time_t timeout)
{
	// From: https://stackoverflow.com/questions/2917881/how-to-implement-a-timeout-in-read-function-call

	// Create a poll request
	int fd = (long int) serial;
	struct pollfd pollfd =
	{
		.fd		= (long int) serial,
		.events	= POLLIN
	};

	// Loop until all data has been read in, or a timeout occurred.
	// TODO(Barach): This timeout is only approximate, should by decreased after each loop by the execution time.
	size_t index = 0;
	while (poll (&pollfd, 1, timeout) == 1)
	{
		#if DEBUGGING
		printf ("Read: ");
		#endif // DEBUGGING

		ssize_t readCount = read (fd, data + index, size);
		if (readCount == -1)
		{
			#if DEBUGGING
			printf ("Error.\n");
			#endif // DEBUGGING
			return false;
		}

		#if DEBUGGING
		for (size_t i = 0; i < readCount; ++i)
			printf ("%02X ", ((uint8_t*) data) [index + i]);
		printf ("\n");
		#endif // DEBUGGING

		index += readCount;
		size -= readCount;

		if (size == 0)
			return true;
	}

	#if DEBUGGING
	printf ("Timeout.\n");
	#endif // DEBUGGING

	return false;
}

#else

// Windows Implementation -----------------------------------------------------------------------------------------------------

#endif // _WIN32