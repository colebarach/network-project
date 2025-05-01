// Includes
#include "/home/joshuat/network-project/src/network_utils/adapter.h"
#include "/home/joshuat/network-project/src/network_utils/serial.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/file.h>

#define DATA_CONTROL 0x69
#define ACK_CONTROL 0x6A
#define RAWDATA_SIZE 1020

int main(int argc, char* argv[])
{
    if (argc != 4)
	{
        fprintf (stderr, "Invalid arguments. Usage: 'rx <src addr> <dest addr> <serial port>'.\n");
		return -1;
	}

    // const char* serialPath = "/dev/ttyS1";
    void* serial = serialInit(argv[3]);
    if (serial == NULL)
    {
        int code = errno;
        fprintf (stderr, "Failed to open serial port '%s': %s.\n", argv[3], strerror (code));
		return code;
    }    

    //argv[1] is the destination name of whatever we're reading from
    char dest_addr[ADDRESS_SIZE + 1] = {};
    strcpy (dest_addr, argv [2]);

    // Set the device address
    setAddress (serial, dest_addr);

    char src_addr[ADDRESS_SIZE + 1] = {};
    strcpy(src_addr, argv[1]);
    
    //temporary, should implement the maxseqnum function. This is assuming RWS = SWS = 1
    const int RWS = 1;
    const int maxSeqNum = 2;

    uint8_t seqNum = 0;

    while(1)
    {
        char segment[DATAGRAM_SIZE];

        int size = 3;

        segment[0] = DATA_CONTROL;
        segment[1] = seqNum;
        segment[2] = size >> 2;
        segment[3] = ((size && 0b0000000011) << 6) + 0b001111; //checksum here, just too lazy to implement it yet

        do
        {
            size++;
            segment[size] = getchar();
        } while (segment[size] != '\n' && size < RAWDATA_SIZE);

        size++;
        transmit(serial, segment, size, src_addr);

        sleep(1); //temporary sleep

        //get ack here!
        seqNum = ++seqNum % 2; //only increase seqnum if you have gotten an ack back

    }

    return 0;
}