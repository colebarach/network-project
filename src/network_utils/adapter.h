#ifndef COMMON_H
#define COMMON_H

// C Standard Library
#include <stdint.h>
#include <stdlib.h>

#define ADDRESS_SIZE 8
#define DATAGRAM_SIZE 1024

/**
 * @brief Sets the address of an adapter.
 * @param serial The serial port of the adapter.
 * @param addr The address to set.
 */
void setAddress (void* serial, const void* addr);

/**
 * @brief Transmits a datagram via an adapter.
 * @param serial The serial port of the adapter.
 * @param payload The payload of the datagram.
 * @param size The size of the payload. Note that this value will be rounded up to the nearest multiple of 4.
 * @param destAddr The address of the destination adapter.
 */
void transmit (void* serial, const void* payload, uint16_t size, const void* destAddr);

/**
 * @brief Receives a datagram from an adapter.
 * @param serial The serial port of the adapter.
 * @param payload Buffer to write the received payload into.
 * @param srcAddr Buffer to write the source address into.
 * @param timeout The amount of time to timeout after. Use -1 to indicate no timeout should be used.
 * @return The size of the received payload.
 */
size_t receive (void* serial, void* payload, char* srcAddr, time_t timeout);

#endif // COMMON_H