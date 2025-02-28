## Transport Layer
| Field     | Size (Bytes)   | Offset (Bytes) | Description                                                                   |
|-----------|----------------|----------------|-------------------------------------------------------------------------------|
| Control   | 1              | 0              | Indicates the type of frame being sent.                                       |
| Seq Num   | 1              | 1              | Frame sequence number, used for ARQ.                                          |
| Padding   | 2              | 2              | Padding data, ignored.                                                        |
| Payload   | 4 * Size       | 4              | The data of the frame.                                                        |

## Network Layer
| Field     | Size (Bytes)   | Offset (Bytes) | Description                                                                   |
|-----------|----------------|----------------|-------------------------------------------------------------------------------|
| Src Addr  | 1              | 0              | The address of the transmitting device.                                       |
| Dest Addr | 1              | 1              | The address of the receiving device.                                          |
| Payload   | N              | 2              | The data of the frame.                                                        |
| Checksum  | 4              | N + 2          | The 32-bit checksum of the payload.                                           |