// Wire Master Reader
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Reads data from an I2C/TWI slave device
// Refer to the "Wire Slave Sender" example for use with this

// Created 29 March 2006

// This example code is in the public domain.


#include <Wire.h>

#define SGP_ADDR 0x58

void setup()
{
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output
  Serial.println("Starting");
}

void loop()
{
  Wire.beginTransmission(SGP_ADDR); // transmit to device
  Wire.write(0x2008);
  Wire.endTransmission(1);
  delay(12);
  Wire.requestFrom(SGP_ADDR, 6);    // request 6 bytes from slave device #2

  while(Wire.available())    // slave may send less than requested
  { 
    char c = Wire.read(); // receive a byte as character
    Serial.print(c);         // print the character
  }

  delay(988);
}
