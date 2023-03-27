#ifndef SGP30
#define SGP30


#include <Wire.h>
#include <stdint.h>
#include "crc.h"


#define ADDR 0x58


static const uint16_t iaq_init = 0x2003, measure_iaq = 0x2008, get_iaq_baseline = 0x2015, set_iaq_baseline = 0x201e;
static uint16_t baseline_CO2, baselineTVOC;


void sgp30_setup() {
  #ifndef WIRE_BEGAN 
  Wire.begin();
  #define WIRE_BEGAN
  #endif

  // Initialize the sensor
  Wire.beginTransmission(ADDR);
  Wire.write(iaq_init);
  Wire.endTransmission();
  delay(10);

  // Send measure_iaq commands at 1sec intervals for 15 seconds
  for (int i = 0; i <= 15; i++) {
    Wire.beginTransmission(ADDR);
    Wire.write(measure_iaq);
    Wire.endTransmission(false);

    Wire.requestFrom(ADDR, 6);
    delay(12);
    while (Wire.available()) Wire.read();

    delay(1000);
  }

  // Obtain the measured baseline values to skip calibration process later
  while (1) {
    Wire.beginTransmission(ADDR);
    Wire.write(get_iaq_baseline);
    Wire.endTransmission(false);
    
    Wire.requestFrom(ADDR, 6);
    delay(10);

    // Restart request if less than 6 bytes available
    if (Wire.available() < 6) continue;
    uint8_t response[6];
    for (int i = 0; i < 6; i++) {
      response[i] = Wire.read();
    }

    // Restart request if either CRC check fails
    if (check_crc(&response[0])) continue;
    if (check_crc(&response[3])) continue;

    // Store calibrated baseline values
    memcpy(&baseline_CO2, &response[0], 2);
    memcpy(&baselineTVOC, &response[3], 2);
    break;
  }
}


uint16_t sgp30_getCO2() {
  // Turn on sensor
  Wire.beginTransmission(ADDR);
  Wire.write(iaq_init);
  Wire.endTransmission();
  delay(10);

  // Write calibration values
  Wire.beginTransmission(ADDR);
  Wire.write(set_iaq_baseline);
  Wire.write(baselineTVOC);
  Wire.write(calculate_crc(&baselineTVOC));
  Wire.write(baseline_CO2);
  Wire.write(calculate_crc(&baseline_CO2));
  Wire.endTransmission();
  delay(10);

  // Perform measurement
  while (1) {
    Wire.beginTransmission(ADDR);
    Wire.write(measure_iaq);
    Wire.endTransmission(false);

    Wire.requestFrom(ADDR, 6);
    delay(12);

    if (Wire.available() < 6) continue;
    uint8_t response[6];
    for (int i = 0; i < 6; i++) {
      response[i] = Wire.read();
    }

    // Restart request if either CRC check fails
    if (check_crc(&response[0])) continue;
    if (check_crc(&response[3])) continue;

    uint16_t co2;
    memcpy(&co2, &response[0], 2);
    return co2;
  }
}


#endif