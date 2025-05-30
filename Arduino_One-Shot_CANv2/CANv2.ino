// This program and the accompanying library "Canbus" included in the project folder
// are used to send one-shot CAN messages at 400kHz when an input pin is toggled.
// To change the bitrate the library file mcp2515.c must be edited. There is a 
// website that is used to figure out the register values needed.
// CAN Bus Bit Timing Calculator - https://kvaser.com/support/calculators/bit-timing-calculator/
// Built with Arduino IDE 2.3.3

#include <Canbus.h>
#include <defaults.h>
#include <global.h>
#include <mcp2515.h>
#include <mcp2515_defs.h>
const int passwordPin = 2;
const int addressPin = 3;
void setup() {
  pinMode(passwordPin, INPUT);
  pinMode(addressPin, INPUT);
  Serial.begin(115200);
  Serial.println("CAN Write - Testing transmission of CAN Bus messages");
  delay(1000);
  if(Canbus.init(CANSPEED_500)) 
//Initialise MCP2515 CAN controller at the specified speed
  Serial.println("CAN Init ok");
  else
  Serial.println("Can't init CAN");
  delay(1000);
}
void loop()
{
tCAN password_message;
        password_message.id = 0x011; //formatted in HEX
        password_message.header.rtr = 0;
        password_message.header.length = 8; //formatted in DEC
        password_message.data[0] = 0xFE;
        password_message.data[1] = 0xED;
        password_message.data[2] = 0xFA;
        password_message.data[3] = 0xCE; //formatted in HEX
        password_message.data[4] = 0xCA;
        password_message.data[5] = 0xFE;
        password_message.data[6] = 0xBE;
        password_message.data[7] = 0xEF;
tCAN address_message;
        address_message.id = 0x012; //formatted in HEX
        address_message.header.rtr = 0;
        address_message.header.length = 8; //formatted in DEC
        address_message.data[0] = 0x40;
        address_message.data[1] = 0x00;
        address_message.data[2] = 0x00;
        address_message.data[3] = 0x00; //formatted in HEX
        address_message.data[4] = 0x00;
        address_message.data[5] = 0x00;
        address_message.data[6] = 0xD0;
        address_message.data[7] = 0x00;
mcp2515_bit_modify(CANCTRL, (1<<REQOP2)|(1<<REQOP1)|(1<<REQOP0), 0);
mcp2515_bit_modify(CANCTRL, (1<<OSM), 8);
while(true) {
  if(digitalRead(passwordPin) == LOW) {
    mcp2515_send_message(&password_message);
    delay(10);
  }
  if(digitalRead(addressPin) == LOW) {
    mcp2515_send_message(&address_message);
    delay(10);
  }
}
}
