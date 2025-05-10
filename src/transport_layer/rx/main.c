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

    //my sliding window number starts at 0
    int seqNum = 0;

    while(1)
    {
        char src_addr[ADDRESS_SIZE + 1];
        uint8_t data[DATAGRAM_SIZE];
        size_t payload_size = receive(serial, data, src_addr, -1); //change the timeout to something
        src_addr [ADDRESS_SIZE] = '\0';
        uint8_t ack_segment[4] = {ACK_CONTROL, 0, 0, 0};

        if (!strcmp(expected_src_addr, src_addr) && seqNum == data[1] && payload_size != 0)
        {
            payload_size = data[2] | (data[3] & 0b11) << 8;
            int sent_checksum = 0;

            //calculating internet checksum. Checksum is based off every four bits, so for each byte, we 
            //use the lower 4 bits, then the upper 4 bits, when calculating the checksum.
            for (int i = 0; i < 2 * (payload_size + 4); i++)
            {
                if (i % 2 == 1)
                {
                    sent_checksum += (data[i/2] & 0b1111);
                }
                else
                {
                    sent_checksum += (data[i/2] & 0b11110000) >> 4;
                }
                if (sent_checksum > 0b1111) //if there is a carried one after the 4th bit, then add the carry to the lowest bit.
                {
                    sent_checksum = (sent_checksum & 0b1111) + 1;
                }
            }

            if (sent_checksum == 0b1111) //based on the how the checksum works, all ones is a correct checksum.
            {
                int control = data[0];
                int receivedSeqNum = data[1];

                //these two are stored in a little endian fashion.
                int internet_checksum = (data[3] & 0b11110000) >> 4;

                fwrite(data + 4, 1, payload_size, stdout);
                fflush(stdout);

                ack_segment[1] = receivedSeqNum;

                int sending_checksum = 0;

                //calculating internet checksum. Checksum is based off every four bits, so for each byte, we 
                //use the lower 4 bits, then the upper 4 bits, when calculating the checksum.
                for (int i = 0; i < 2 * (4); i++)
                {
                    if (i % 2 == 1)
                    {
                        sending_checksum += (ack_segment[i/2] & 0b1111);
                    }
                    else
                    {
                        sending_checksum += (ack_segment[i/2] & 0b11110000) >> 4;
                    }
                    if (sending_checksum > 0b1111) //if there is a carried one after the 4th bit, then add the carry to the lowest bit.
                    {
                        sending_checksum = (sending_checksum & 0b1111) + 1;
                    }
                }

                sending_checksum = ~sending_checksum; //inverting the bits is the last step of calculating the checksum.

                sending_checksum <<= 4;
                ack_segment[3] = sending_checksum; //don't include data as we know it's going to be 0000.

                transmit(serial, ack_segment, 4, expected_src_addr);
                seqNum = ++seqNum % 2;
            }
        }
        
        //case of wrong sequence number, AKA the previously sent segment being received again
        else if (!strcmp(expected_src_addr, src_addr) && seqNum != data[1] && payload_size != 0)
        {
            ack_segment[1] = data[1];

            int sending_checksum = 0;

            for (int i = 0; i < 2 * (4); i++)
            {
                if (i % 2 == 1)
                {
                    sending_checksum += (ack_segment[i/2] & 0b1111);
                }
                else
                {
                    sending_checksum += (ack_segment[i/2] & 0b11110000) >> 4;
                }
                if (sending_checksum > 0b1111)
                {
                    sending_checksum = (sending_checksum & 0b1111) + 1;
                }
            }

            sending_checksum = ~sending_checksum;

            sending_checksum <<= 4;
            ack_segment[3] = sending_checksum;

            transmit(serial, ack_segment, 4, expected_src_addr);
        }
    }

    return 0;
}

