# Inter-layer Interfaces
This file describes the interface between each layer of the network stack. A program implementing one or more layers of the
stack must implement this interface correctly to guarantee compatibility.

Note that all data is in the little-endian format.

## The Application Layer / Transport Layer Interface
The application layer and transport layers are connected via a data stream.

For the transmit application, the frame to transmit should be written to the standard input stream. After the last byte has
been written, the stream should be closed.

For the receive application, the first frame that was received will be written to the standard output stream. The application
will continue to write received frames until terminated.

## The Transport Layer / Network Layer Interface
The transport layer and network layer are connected via a serial port. Commands are issued to the device to configure and
transmit data. Responses are delivered when data is received.

### Transmit Command
To transmit a frame, the transmit command should be issued including the frame to be written. The type byte indicates the type
of command being issued, and, by extension, the format of the command. After the destination address, the size of the frame
should be sent. This indicates the remaining length of the command.

| Field     | Size (Bytes)   | Offset (Bytes) | Description                                                                   |
|-----------|----------------|----------------|-------------------------------------------------------------------------------|
| Type      | 1              | 0              | The type of the sequence (0x7C for transmit)                                  |
| Dest Addr | 8              | 1              | The address of the receiving device.                                          |
| Size      | 1              | 9              | The size of the payload, in 4 byte blocks.                                    |
| Frame     | 4 * (Size + 1) | 10             | The frame to transmit.                                                        |

### Receive Response
When a frame is received, the receive response is written. The type byte indicates the type of response being issued, and, by
extension, the format of the response. After the source address, the size of the frame is written. This indicates the
remaining length of the command.

| Field     | Size (Bytes)   | Offset (Bytes) | Description                                                                   |
|-----------|----------------|----------------|-------------------------------------------------------------------------------|
| Type      | 1              | 0              | The type of the sequence (0x7D for receive)                                   |
| Src Addr  | 8              | 1              | The address of the transmitting device.                                       |
| Size      | 1              | 9              | The size of the payload, in 4 byte blocks.                                    |
| Frame     | 4 * (Size + 1) | 10             | The received frame.                                                           |