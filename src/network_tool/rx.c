// POSIX
#include <fcntl.h>
#include <termio.h>
#include <unistd.h>
#include <sys/file.h>

// C Standard Library
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

int main (int argc, char** argv)
{
	if (argc != 2)
	{
		fprintf (stderr, "Invalid arguments. Usage: 'rx <serial port name>'.\n");
		return -1;
	}

	const char* serialPath = argv [1];

	int serial = open (serialPath, O_RDONLY);
	if (serial < 0)
	{
		int code = errno;
		fprintf(stderr, "Failed to open serial port '%s': %s.\n", serialPath, strerror (code));
		return code;
	}

	if (flock (serial, LOCK_EX | LOCK_NB) == -1)
	{
		close (serial);

		int code = errno;
		printf ("Failed to lock serial port '%s': %s.\n", serialPath, strerror (code));
		return code;
	}

	struct termios tty;
	if (tcgetattr (serial, &tty) != 0)
	{
		int code = errno;
		printf ("Failed to get TTY attributes: %s.\n", strerror (errno));
		return code;
	}

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

	if (tcsetattr (serial, TCSANOW, &tty) != 0)
	{
		int code = errno;
		printf("Failed to set TTY attributes: %s.\n", strerror(errno));
		return code;
	}

	close (serial);

	// TODO(Barach): For some reason, this is the only way I can open a serial port properly.
	FILE* fSerial = fopen (serialPath, "r");
	if (fSerial == NULL)
	{
		int code = errno;
		fprintf (stderr, "Failed to open serial port '%s': %s.\n", serialPath, strerror (code));
		return code;
	}

	uint8_t address [9];
	char data [1025];

	while (1)
		if (getc (fSerial) == 0x7D)
			break;

	for (uint8_t index = 0; index < 8; ++index)
		address [index] = getc (fSerial);
	address [8] = '\0';

	uint8_t size = (getc (fSerial) + 1) * 4;

	for (uint16_t index = 0; index < size; ++index)
		data [index] = getc (fSerial);
	data [1024] = '\0';

	printf ("Message: %s\n", data);
	printf ("From: %s\n", address);

	fclose (fSerial);
}