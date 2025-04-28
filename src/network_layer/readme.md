# Usage
## Configuration
- Create a directory named ```build```.
- From said directory, run ```cmake ..```.
- To generate debug symbols, use ```cmake -DCMAKE_BUILD_TYPE=Debug ..```
## Compilation
- From the build directory, run ```make```.
## Programming
- From the build directory, run
``` openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "program build/main.elf verify reset exit"```

# Documentation

Datagram Format:
| Field     | Size (Bytes)   | Offset (Bytes) | Description                                                                   |
|-----------|----------------|----------------|-------------------------------------------------------------------------------|
| Src Addr  | 8              | 0              | The address of the transmitting device.                                       |
| Dest Addr | 8              | 8              | The address of the receiving device.                                          |
| Payload   | N              | 16             | The data of the frame.                                                        |