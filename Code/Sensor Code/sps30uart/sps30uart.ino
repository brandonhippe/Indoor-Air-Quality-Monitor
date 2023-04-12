#include <SoftwareSerial.h>
#include <stdint.h>
#include <stdlib.h>


#define rx 12
#define tx 11
SoftwareSerial sps30(rx, tx);


uint8_t data[263];


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
  Serial.begin(9600);
  sps30.begin(115200);

  Serial.println("Starting");
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    if (input.length() % 2 == 1) {
      input = '0' + input;
    }

    for (int i = 0; i < input.length(); i += 2) {
      data[i >> 1] = strtol(input.substring(i, i + 2).c_str(), NULL, 16);
    }

    Serial.print("Transmitted message: 0x");
    printHex(data, input.length() >> 1);
    Serial.println();

    sps30.write(data, input.length() >> 1);

    while (sps30.available() == 0);
    
    int i = 0;
    while (sps30.available()) {
      data[i] = sps30.read();
      i++;
    }

    Serial.print("Received message: 0x");
    printHex(data, i);
    Serial.println();
  }
}
