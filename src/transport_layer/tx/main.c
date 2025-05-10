// Includes
#include "../../network_utils/adapter.h"
#include "../../network_utils/serial.h"

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


// argv[1]: source address
// argv[2]: destination address
// argv[3]: Linux serial port address
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

    //argv[1] is the destination name of whatever we're reading from
    char dest_addr[ADDRESS_SIZE + 1] = {};
    strcpy (dest_addr, argv [2]);

    char src_addr[ADDRESS_SIZE + 1] = {};
    strcpy(src_addr, argv[1]);

    // Set the device address
    setAddress (serial, src_addr);

    const int RWS = 1;
    const int maxSeqNum = 2;

    uint8_t seqNum = 0;



    while(1)
    {
        uint8_t segment[DATAGRAM_SIZE];

        size_t size = 0;

        segment[0] = DATA_CONTROL;
        segment[1] = seqNum;

        for (; size < RAWDATA_SIZE; ++size)
        {
			int dataByte = getchar(); //This is where we receive the data from stdin.

			if (dataByte == -1) //error case where an EOF is encountered.
				return 0;

            segment[size + 4] = dataByte;

            if (dataByte == '\n') //stop reading when endline is read.
                break;
        }
        ++size;

        segment[2] = size;
        segment[3] = (size >> 8); //the most significiant bits of the size are put in the right most two bits. We do this in order to adhere to little-endianness.

        uint8_t sending_checksum = 0;

        //calculating internet checksum. Checksum is based off every four bits, so for each byte, we 
        //use the lower 4 bits, then the upper 4 bits, when calculating the checksum.
        for (int i = 0; i < 2 * (size + 4); i++)
        {
            if (i % 2 == 1)
            {
                sending_checksum += (segment[i/2] & 0b1111);
            }
            else
            {
                sending_checksum += (segment[i/2] & 0b11110000) >> 4;
            }
            if (sending_checksum > 0b1111) //if there is a carried one after the 4th bit, then add the carry to the lowest bit.
            {
                sending_checksum = (sending_checksum & 0b1111) + 1;
            }
        }

        sending_checksum = ~sending_checksum; //inverting the bits is the last step of calculating the checksum.

        //the internet checksum is stored in the leftmost 4 bits. We do this in order to adhere to little-endianness.
        segment[3] = segment[3] | (sending_checksum << 4);

        int failedattempts = 0;
        while(1)
        {
            transmit(serial, segment, size + 4, dest_addr); //send segment
            char reported_dest_addr[ADDRESS_SIZE + 1];
            char data[DATAGRAM_SIZE];
            size_t ackSize = receive(serial, data, reported_dest_addr, 50); //wait 50ms for ACK segment
            reported_dest_addr [ADDRESS_SIZE] = '\0';
            if (!strcmp(reported_dest_addr, dest_addr) && seqNum == data[1] && data[0] == ACK_CONTROL && ackSize == 4)
            {
                int ack_checksum = 0;

                for (int i = 0; i < 2 * (4); i++)
                {
                    if (i % 2 == 1)
                    {
                        ack_checksum += (data[i/2] & 0b1111);
                    }
                    else
                    {
                        ack_checksum += (data[i/2] & 0b11110000) >> 4;
                    }
                    if (ack_checksum > 0b1111)
                    {
                        ack_checksum = (ack_checksum & 0b1111) + 1;
                    }
                }

                if (ack_checksum == 0b1111) //checksum matches, thus ACK segment is acknowledged.
                    break;
            }
            failedattempts++;
            if (failedattempts == 16)
            {
                fprintf(stderr, "Failed to transmit: Too many transmission attempts made. Number of attempted transmissions: %i\n", failedattempts);
                return -1;
            }
        }

        //get ack here! transmit again if ack doesn't happen
        seqNum = ++seqNum % 2; //only increase seqnum if you have gotten an ack back

    }

    return 0;
}