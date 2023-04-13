#include <Wire.h>


// Define I2C Address
#define ADDR 0x58

// Define bytes to send as response to request(s)
const uint8_t bytes[] = { 0x01, 0x90, 0x4C, 0x00, 0x00, 0x81,
                          0x01, 0x90, 0x4C, 0x00, 0x00, 0x81,
                          0x01, 0x90, 0x4C, 0x00, 0x00, 0x81,
                          0x01, 0x90, 0x4C, 0x00, 0x00, 0x81,
                          0x01, 0x90, 0x4C, 0x00, 0x00, 0x81,
                          0x01, 0x90, 0x4C, 0x00, 0x00, 0x81,
                          0x01, 0x90, 0x4C, 0x00, 0x00, 0x81,
                          0x01, 0x90, 0x4C, 0x00, 0x00, 0x81,
                          0x01, 0x90, 0x4C, 0x00, 0x00, 0x81,
                          0x01, 0x90, 0x4C, 0x00, 0x00, 0x81,
                          0x01, 0x90, 0x4C, 0x00, 0x00, 0x81,
                          0x01, 0x90, 0x4C, 0x00, 0x00, 0x81,
                          0x01, 0x90, 0x4C, 0x00, 0x00, 0x81,
                          0x01, 0x90, 0x4C, 0x00, 0x00, 0x81,
                          0x01, 0x90, 0x4C, 0x00, 0x00, 0x81,
                          0x00, 0xDA, 0x5E, 0x00, 0x45, 0x49,
                          0x00, 0xDA, 0x5E, 0x00, 0x45, 0x49  };
const int response_ends[] = {0, 6, 12, 18, 24, 30, 36, 42, 48, 54, 60, 66, 72, 78, 84, 90, 96, 102};
const int num_responses = 17;
int response_ix = 0;


void printHex(uint8_t bytes[], int nBytes) {
  for (int i = 0; i < nBytes; i++) {
    Serial.print(' ');
    if (bytes[i] < 0x10) {
      Serial.print('0');
    }

    Serial.print(bytes[i], HEX);
  }

  Serial.println();
}


void setup() {
  // Join I2C Bus and register receive and request events
  Wire.begin(ADDR);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  
  Serial.begin(9600);           // start serial for output
  Serial.println("Starting");
}

void loop() {
}


void receiveEvent(int bytes) {
  String data;
  while (Wire.available()) {
    data += Wire.read();
  }

  Serial.print("Received Message: 0x");
  printHex((uint8_t *)data.c_str(), data.length());
  Serial.println();
}


void requestEvent() {
  Wire.write(&bytes[response_ix], response_ends[response_ix + 1] - response_ends[response_ix]);
  response_ix += 1;
  response_ix = response_ix % num_responses;
}
