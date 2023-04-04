#include "sgp30.h"


// Blocking setup code
void sgp30_setup() {
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


// Non-blocking setup functions
void sgp30_setup_start(uint64_t currTime_ms) {
  uint32_t startTime = millis();
  
  // Initialize the sensor
  Wire.beginTransmission(ADDR);
  Wire.write(iaq_init);
  Wire.endTransmission();

  // Schedule Calibration Measurement
  calibrationMeasurements = 0;
  sgp30_scheduledFunc = sgp30_calibration;
  sgp30_time_ms = currTime_ms + 100 + startTime - millis();
}

void sgp30_calibration(uint64_t currTime_ms) {
  uint32_t startTime = millis();
  
  // Perform a new calibration measurement if 15 or less have been performed
  if (calibrationMeasurements <= 15) {
    calibrationMeasurements++;
    Wire.beginTransmission(ADDR);
    Wire.write(measure_iaq);
    Wire.endTransmission(false);

    Wire.requestFrom(ADDR, 6);
    delay(12);
    while (Wire.available()) Wire.read();

    // Schedule new calibration measurement
    sgp30_time_ms = currTime_ms + 1000 + startTime - millis();
    return;
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

  // Schedule measurement request
  sgp30_scheduledFunc = sgp30_getCO2;
  sgp30_time_ms = currTime_ms + sgp30_period_ms + startTime - millis();
}


void sgp30_getCO2(uint64_t currTime_ms) {
  uint32_t startTime = millis();
  
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

    memcpy(&sgp30_co2, &response[0], 2);
  }

  // Schedule new measurement request
  sgp30_scheduledFunc = sgp30_getCO2;
  sgp30_time_ms = currTime_ms + sgp30_period_ms + startTime - millis();
}
