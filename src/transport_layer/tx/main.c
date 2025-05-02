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

    char src_addr[ADDRESS_SIZE + 1] = {};
    strcpy(src_addr, argv[1]);

    // Set the device address
    setAddress (serial, src_addr);
    
    //temporary, should implement the maxseqnum function. This is assuming RWS = SWS = 1
    const int RWS = 1;
    const int maxSeqNum = 2;

    uint8_t seqNum = 0;



    while(1)
    {
        uint8_t segment[DATAGRAM_SIZE];

        int size = 3;

        segment[0] = DATA_CONTROL;
        segment[1] = seqNum;

        do
        {
            size++;
            segment[size] = getchar();
        } while (segment[size] != '\n' && size < RAWDATA_SIZE);

        size++;

        // segment[2] = size >> 2;
        segment[2] = size - 4;
        segment[3] = (size >> 8) | 0b11110000; //checksum here, just too lazy to implement it yet. checksum is 0b1111 but put on the upper 4 bits

        // sleep(1); //temporary sleep

        int failedattempts = 0;
        while(1)
        {
            transmit(serial, segment, size, dest_addr);
            char reported_dest_addr[ADDRESS_SIZE];
            char data[DATAGRAM_SIZE];
            uint8_t payload_size = receive(serial, data, reported_dest_addr, 50); //change the timout to something
            if (!strcmp(reported_dest_addr, dest_addr) && seqNum == data[1] && data[0] == ACK_CONTROL && payload_size == 4)
            {
                //do internet checksum
                break;
            }
            else {
                failedattempts++;
                if (failedattempts == 16)
                {
                    fprintf(stderr, "Failed to transmit: Too many transmission attempts made. Number of attempted transmissions: %i\n", failedattempts);
                    return -1;
                }
            }            
        }

        //get ack here! transmit again if ack doesn't happen
        seqNum = ++seqNum % 2; //only increase seqnum if you have gotten an ack back

    }

    return 0;
}