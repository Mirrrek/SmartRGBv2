# SmartRGBv2
Firmware for an ESP32-based LED light strip controller. Connects to a local network and communicated over UDP.

# Circuit
Connect pins 16, 17 and 21 to the RGB strip **through a transistor and resistor** (else you'll fry your GPIO). Connect the fourth pin of your RGB strip to either GND or your external power supply (depends if the strips are common-anode or common-cathode). **Make sure to connect your external power supply GND to your ESP32 GND.**

# UDP Protocol
Every UDP packet is composed of a packet code and the packet data.  

### Client to Server
| Packet code | Packet name  | Packet data | Packet description                                                        |
|:------------|:-------------|:------------|:--------------------------------------------------------------------------|
| `0x00`      | `PING`       | none        | Requests a PONG from the server.                                          |
| `0x10`      | `GET`        | none        | Requests the current RGB values from the server.                          |
| `0x20`      | `SET`        | `[r,g,b]`   | Sets the current color.                                                   |
| `0x30`      | `TRANSITION` | `[r,g,b,t]` | Starts a transition from the current color to a goal color for t seconds. |
| `0x40`      | `BURST`      | `[r,g,b]`   | Sets the current color (optimized for realtime color streaming).          |

### Server to Client
| Packet code | Packet name      | Packet data | Packet description                                                                                      |
|:------------|:-----------------|:------------|:--------------------------------------------------------------------------------------------------------|
| `0x01`      | `PONG`           | none        | Responds to a PING.                                                                                     |
| `0x11`      | `GET_RES`        | `[r,g,b]`   | Returns the current RGB values.                                                                         |
| `0x21`      | `SET_RES`        | `[r,g,b]`   | Responds to SET, returns the goal color (for control).                                                  |
| `0x31`      | `TRANSITION_RES` | `[r,g,b,t]` | Responds to TRANSITION, returns the goal color and time (for control).                                  |
| `0xff`      | `ERROR`          | none        | Indicates an error has been thrown processing the packet (invalid packet code / invalid packet length). |

### Packet examples (C notation):  
Get the current color:  
`uint8_t *data = { 0x10 };`  
`int dataLen = 1;`  
  
Set the current color to #007fff:  
`uint8_t *data = { 0x20, 0x00, 0x7f, 0xff };`  
`int dataLen = 4;`  
  
Transition from the current color to #ff00a0 for 15 seconds:  
`uint8_t *data = { 0x30, 0xff, 0x00, 0xa0, 15 };`  
`int dataLen = 5;`  
