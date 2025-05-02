Segment Format:
| Field     | Size (Bits)   | Offset (Bits)  | Description                                                                         |
|-----------|---------------|----------------|-------------------------------------------------------------------------------      |
| Control   | 8             | 0              | Indicates the type of frame being sent.                                             |
| Seq Num   | 8             | 8              | Frame sequence number, used for ARQ.                                                |
| Size      | 10            | 16             | The size of the payload, in bytes.                                                  |
| Checksum  | 6             | 26             | Internet checksum to verify data integrity (ignore first two most significant bits?)|
| Payload   | Size          | 32             | The data of the frame.                                                              |

Control Byte Types:
0x69:Data Segment
0x6A:ACK Segment