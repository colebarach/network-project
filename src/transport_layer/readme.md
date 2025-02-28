Segment Format:
| Field     | Size (Bytes)   | Offset (Bytes) | Description                                                                   |
|-----------|----------------|----------------|-------------------------------------------------------------------------------|
| Control   | 1              | 0              | Indicates the type of frame being sent.                                       |
| Seq Num   | 1              | 1              | Frame sequence number, used for ARQ.                                          |
| Size      | 2              | 2              | The size of the payload, in bytes.                                            |
| Payload   | 4 * Size       | 4              | The data of the frame.                                                        |