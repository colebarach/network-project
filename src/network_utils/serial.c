// Header
#include "serial.h"

#ifndef _WIN32

// POSIX Implementation -------------------------------------------------------------------------------------------------------

// C Standard Library
#include <stdio.h>

// POSIX
#include <fcntl.h>
#include <termio.h>
#include <unistd.h>
#include <sys/file.h>

void* serialInit (const char* port)
{
	// Open and configure the port as a TTY.

	int fd = open (port, O_RDONLY);
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
	tty.c_iflag &= ~(IXON | IXOFF | IXANY); 	// Turn off s/w flow ctrl
	tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | // Disable any special handling of received bytes
		ISTRIP | INLCR | IGNCR | ICRNL);
	tty.c_oflag &= ~OPOST;						// Prevent special interpretation of output bytes (e.g. newline chars)
	tty.c_oflag &= ~ONLCR;						// Prevent conversion of newline to carriage return/line feed

	tty.c_cc[VMIN] = 1;
	tty.c_cc[VTIME] = 0;

	cfsetispeed(&tty, B9600);
	cfsetospeed(&tty, B9600);

	cfmakeraw (&tty);

	if (tcsetattr (fd, TCSANOW, &tty) != 0)
		return NULL;

	// TODO(Barach): This is the only way I can get serial ports to work.
	// Close the port and re-open it via the C standard library.
	close (fd);
	return fopen (port, "r+");
}

void serialClose (void* serial)
{
	fclose (serial);
}

void serialWrite (void* serial, const void* data, size_t size)
{
	fwrite (data, sizeof (data), size, serial);
}

size_t serialRead (void* serial, void* data, size_t size)
{
	return fread (data, 1, size, serial);
}

#else

// Windows Implementation -----------------------------------------------------------------------------------------------------

#endif // _WIN32