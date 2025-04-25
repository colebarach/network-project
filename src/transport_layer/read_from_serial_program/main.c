#include <stdio.h>
#include <stdlib.h>
// #include <fcntl.h>
#include <io.h>

//program to write to serial port
int main(int argc, char* argv[])
{
    //argv[1] is the destination name of whatever we're reading from
    
    
    //
    //

    int type = getchar();

    int src_addr[8];
    for (int i = 0; i < 8; i++)
    {
        src_addr[i] = getchar();
    }

    const int size = 4 * (getchar() + 1);
    int data[size];
    //just do byte by byte until you hit the size given from serial, which is a multiple of 4 for bytes plus 1.
    for (int i = 0; i < size; i++)
    {
        data[i] = getchar();
    }

    return 0;
}