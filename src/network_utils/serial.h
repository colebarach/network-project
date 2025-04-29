#ifndef SERIAL_H
#define SERIAL_H

// C Standard Library
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

/**
 * @brief Opens and initializes a serial port using the operating-system-specific method.
 * @param port The identifier of the serial port to open. For POSIX, use the file path, ex. '/dev/ttyACM0'. For Windows, use
 * the COM port name, ex 'COM1'.
 * @return The handler for the serial port, if successful, NULL otherwise.
 */
void* serialInit (const char* port);

/**
 * @brief Closes a previously opened serial port using the operating-system-specific method.
 * @param serial The serial port handler, as returned by @c serialInit .
 */
void serialClose (void* serial);

/**
 * @brief Writes @c size bytes to a serial port.
 * @param serial The serial port handler, as returned by @c serialInit .
 * @param data The buffer of data to write.
 * @param size The number of bytes to write.
 */
void serialWrite (void* serial, const void* data, size_t size);

/**
 * @brief Reads @c size bytes from a serial port.
 * @param serial The serial port handler, as returned by @c serialInit .
 * @param data The buffer to read data into.
 * @param size The number of elements to read.
 * @param timeout The amount of time to time out after.
 * @return True if all of the data was read, false if a timeout or error occurred.
 */
bool serialRead (void* serial, void* data, size_t size, time_t timeout);

#endif // SERIAL_H