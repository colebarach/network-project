#ifndef SERIAL_H
#define SERIAL_H

// C Standard Library
#include <stdint.h>
#include <stdlib.h>

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
 * @return The number of elements that were actually read. If not equal to size, a timeout or error occurred.
 */
size_t serialRead (void* serial, void* data, size_t size);

#endif // SERIAL_H