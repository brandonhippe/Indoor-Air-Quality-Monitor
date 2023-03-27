#ifndef SPS30
#define SPS30


#include <Wire.h>
#include <stdint.h>
#include "crc.h"


#define ADDR 0x69

#define MCPM1p0 0
#define MCPM2p5 1
#define MCPM4p0 2
#define MCPM10 3
#define NCPM0p5 4
#define NCPM1p0 5
#define NCPM2p5 6
#define NCPM4p0 7
#define NCPM10 8
#define PART_SIZE 9

static const uint16_t start_measurement = 0x0010, stop_measurement = 0x0014, data_ready = 0x0202, read_measured = 0x0300, start_sleep = 0x1001, wakeup = 0x1103;


void sps30_setup() {
  #ifndef WIRE_BEGAN 
  Wire.begin();
  #define WIRE_BEGAN
  #endif

  // Put sensor into sleep
  Wire.beginTransmission(ADDR);
  Wire.write(start_sleep);
  Wire.endTransmission();
}


uint16_t *sps30_getVal(int measurement[], int numVals) {
  // Turn on sensor
  Wire.beginTransmission(ADDR);
  Wire.write(wakeup);
  Wire.endTransmission();
  // Send wakeup again to finish wakeup command
  Wire.beginTransmission(ADDR);
  Wire.write(wakeup);
  Wire.endTransmission();

  // Put into measurement mode
  Wire.beginTransmission(ADDR);
  Wire.write(start_measurement);
  uint8_t init_vals[2] = {0x05, 0x00};
  Wire.write(init_vals[0]); // Set up for 16-bit integer values
  Wire.write(init_vals[1]);
  Wire.write(calculate_crc(&init_vals[0]));
  Wire.endTransmission();
  delay(20);

  // Wait for 16 seconds
  delay(16000);

  // Read and check if all related number concentration > 300/cm^3
  uint16_t checks[5] = {300, 300, 300, 300, 300};
  for (int i = 0; i < numVals; i++) {
    switch (measurement[i]) {
        case NCPM0p5:
            checks[NCPM0p5 - 4] = 0;
        case NCPM1p0:
        case MCPM1p0:
            checks[NCPM1p0 - 4] = 0;
            break;
        case NCPM2p5:
        case MCPM2p5:
            checks[NCPM2p5 - 4] = 0;
            break;
        case NCPM4p0:
        case MCPM4p0:
            checks[NCPM4p0 - 4] = 0;
            break;
        case NCPM10:
        case MCPM10:
            checks[NCPM10 - 4] = 0;
            break;
    }
  }
  
  uint8_t response[30];
  while (1) {
    Wire.beginTransmission(ADDR);
    Wire.write(read_measured);
    Wire.endTransmission();

    Wire.requestFrom(ADDR, 30);
    delay(20);

    if (Wire.available() < 30) continue;
    for (int i = 0; i < 30; i++) {
        response[i] = Wire.read();
    }

    // Restart request if any CRC check fails
    int i;
    for (i = 0; i < 30; i += 3) {
        if (check_crc(&response[i])) break;
    }

    if (i != 30) continue;

    // Obtain number concentration(s)
    for (int i = NCPM0p5; i <= NCPM10; i++) {
        if (checks[i - 4] == 0) {
            memcpy(&checks[i - 4], &response[i * 3], 2); 
        }
    }

    break;
  }

  int checkNum;
  for (checkNum = 0; checkNum < 5; checkNum++) {
    if (checks[checkNum] < 300) {
        break;
    }
  }

  if (checkNum == 5) {
    // All over 300, ready to return values
    uint16_t measurements[numVals];
    for (int i = 0; i < numVals; i++) {
        memcpy(&measurements[i], &response[3 * measurement[i]], 2);
    }

    Wire.beginTransmission(ADDR);
    Wire.write(start_sleep);
    Wire.endTransmission();
    
    return &measurements[0];
  }

  // Wait for another 14 seconds, measure again
  delay(14000);
  
  while (1) {
    Wire.beginTransmission(ADDR);
    Wire.write(read_measured);
    Wire.endTransmission();

    Wire.requestFrom(ADDR, 30);
    delay(20);

    if (Wire.available() < 30) continue;
    for (int i = 0; i < 30; i++) {
        response[i] = Wire.read();
    }

    // Restart request if any CRC check fails
    int i;
    for (i = 0; i < 30; i += 3) {
        if (check_crc(&response[i])) break;
    }

    if (i != 30) continue;

    // Obtain measurement(s)
    uint16_t measurements[numVals];
    for (int i = 0; i < numVals; i++) {
        memcpy(&measurements[i], &response[3 * measurement[i]], 2); 
    }

    Wire.beginTransmission(ADDR);
    Wire.write(start_sleep);
    Wire.endTransmission();
    
    return &measurements[0];
  }
}


#endif
