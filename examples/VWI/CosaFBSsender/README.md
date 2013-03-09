CosaFBSsender
=============
Demonstration of the Virtual Wire Interface (VWI) driver using the
fixed bitstuffing codec (4 to 5 bit symbols). Transmits a simple
message with identity, message number, and two analog data
samples. See CosaFBSreceiver. 

Circuit
-------
Connect RF433/315 Transmitter Data to Arduino(ATtiny85) D12(D2),
VCC to Arduino D10(D1) and connect GND. Connect Arduino analog pins
A2 and A3 to analog sensors. On ATtiny85 the pins are D2, D1. 
The power control pin D10(D1) can also be used for a LED.
