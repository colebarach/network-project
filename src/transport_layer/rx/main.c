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

//program to write to serial port
int main(int argc, char* argv[])
{
	if (argc != 4)
	{
        fprintf (stderr, "Invalid arguments. Usage: 'rx <src addr> <dest addr> <serial port>'.\n");
		return -1;
	}

    void* serial = serialInit(argv[3]);
    if (serial == NULL)
    {
        int code = errno;
        fprintf (stderr, "Failed to open serial port '%s': %s.\n", argv[3], strerror (code));
		return code;
    }

    int ack_segment[4] = {ACK_CONTROL, 0, 0, 0};


    //argv[1] is the destination name of whatever we're reading from
    char dest_addr[ADDRESS_SIZE + 1] = {};
    strcpy (dest_addr, argv [2]);

    // Set the device address
	setAddress (serial, dest_addr);

    char expected_src_addr[ADDRESS_SIZE + 1] = {};
    strcpy(expected_src_addr, argv[1]);

    //temporary, should implement the maxseqnum function. This is assuming RWS = SWS = 1
    const int RWS = 1;
    const int maxSeqNum = 2;

    // int allocated[maxSeqNum];
    // for (int i = 0; i < maxSeqNum; i++)
    // {
    //     allocated[i] = 0;
    // }

    //my sliding window number starts at 0
    int seqNum = 0;
    
    while(1)
    {
        char src_addr[ADDRESS_SIZE];
        char data[DATAGRAM_SIZE];
        uint8_t payload_size = receive(serial, data, src_addr, -1); //change the timout to something

        if (!strcmp(expected_src_addr, src_addr) && seqNum == data[1])
        {    
            //don't forget to use the src_addr somehow. Probably just giving it back to the network layer but also we need to keep track maybe to know
            //which sliding window to use.
            //this next part is the parsing the transport layer segment contained in the data
        
            int control = data[0];
            int seqnum = data[1];
        
            int payload_size = (data[2] << 2) + (data[3] & 0b11000000);
            int internet_checksum = data[3 & 0b00111111];
        
            char payload[payload_size];

            fwrite(payload, 1, payload_size, stdout);
            fflush(stdout);

            //check the internet checksum here

            //make ack packet
            ack_segment[1] = seqNum;


            //internet checksum goes here

            transmit(serial, ack_segment, 4, expected_src_addr);
            seqNum = ++seqNum % 2;
        }
    }



    

    //probably put a while (1) that checks for the next call from the application layer? Or just keep listening until the application layer says no
    //or break while loop after receiving some more serial data? probably that tbh



    #ifdef _WIN32
    
    #endif

    return 0;
}

