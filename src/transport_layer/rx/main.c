#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

//program to write to serial port
int main(int argc, char* argv[])
{
    //argv[1] is the destination name of whatever we're reading from
    
    #ifdef _WIN32
        // This was my source for the Windows stuff: https://www.google.com/search?q=map+serial+port+to+stdin+and+stdout+in+c+on+windows&sca_esv=1708c7095128329f&rlz=1C1ONGR_enUS1147US1147&sxsrf=AHTn8zoZzQJfe_DT2ORU8rQgomrstwTz7Q%3A1745854168177&ei=2J4PaJTNCsjaptQPyKeeoQs&ved=0ahUKEwiUmZjMhfuMAxVIrYkEHciTJ7QQ4dUDCBA&uact=5&oq=map+serial+port+to+stdin+and+stdout+in+c+on+windows&gs_lp=Egxnd3Mtd2l6LXNlcnAiM21hcCBzZXJpYWwgcG9ydCB0byBzdGRpbiBhbmQgc3Rkb3V0IGluIGMgb24gd2luZG93czIFECEYoAEyBRAhGKABMgUQIRigATIFECEYoAEyBRAhGKABSKAOUOICWOQNcAF4AZABAJgBowGgAa4JqgEDMi44uAEDyAEA-AEBmAILoALWCcICChAAGLADGNYEGEfCAgUQIRirApgDAIgGAZAGBpIHAzIuOaAHvTqyBwMxLjm4B9AJ&sclient=gws-wiz-serp
    
    
        // Replace "COM#" with the actual serial port name (e.g., "COM3")
        const char* com_port_name = "\\\\.\\COM3";
        
        // Open the serial port
        HANDLE h_serial = CreateFile(com_port_name,
                                    GENERIC_READ | GENERIC_WRITE,
                                    0,
                                    NULL,
                                    OPEN_EXISTING,
                                    0,
                                    NULL);        


        if (h_serial == INVALID_HANDLE_VALUE) {
            fprintf(stderr, "Error opening serial port: %d\n", GetLastError());
            return 1;
        }

        // Configure serial port settings (baud rate, etc.)
        DCB dcb_serial_params = {0};
        dcb_serial_params.DCBlength = sizeof(dcb_serial_params);

        if (!GetCommState(h_serial, &dcb_serial_params)) {
            fprintf(stderr, "Error getting serial port state: %d\n", GetLastError());
            CloseHandle(h_serial);
            return 1;
        }

        dcb_serial_params.BaudRate = CBR_9600;
        dcb_serial_params.ByteSize = 8;
        dcb_serial_params.StopBits = ONESTOPBIT;
        dcb_serial_params.Parity = NOPARITY;
    
        if (!SetCommState(h_serial, &dcb_serial_params)) {
            fprintf(stderr, "Error setting serial port state: %d\n", GetLastError());
            CloseHandle(h_serial);
            return 1;
        }

        // Redirect stdin and stdout to the serial port
        if (!SetStdHandle(STD_INPUT_HANDLE, h_serial) ||
            !SetStdHandle(STD_OUTPUT_HANDLE, h_serial)) {
            fprintf(stderr, "Error redirecting stdin/stdout: %d\n", GetLastError());
            CloseHandle(h_serial);
            return 1;
        }

        // Now, standard input and output will use the serial port
        printf("Serial port mapped to stdin and stdout.\n");
        printf("Enter text to send: ");

    #elif __linux__
        //do the linux serial port setup
    #endif
    
    //
    //tell adapter what address to listen to first

    //perform this function when prompted

    int type = getchar();
    while (type != 0x7D && type != EOF)
    {
        type = getchar();
    }

    if (type == EOF)
    {
        printf("rip");
    }
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




    #ifdef _WIN32
    
    #endif

    return 0;
}

