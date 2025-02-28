Datagram Format:
| Field     | Size (Bytes)   | Offset (Bytes) | Description                                                                   |
|-----------|----------------|----------------|-------------------------------------------------------------------------------|
| Src Addr  | 1              | 0              | The address of the transmitting device.                                       |
| Dest Addr | 1              | 1              | The address of the receiving device.                                          |
| Payload   | N              | 2              | The data of the frame.                                                        |
| Checksum  | 4              | N + 2          | The 32-bit checksum of the payload.                                           |