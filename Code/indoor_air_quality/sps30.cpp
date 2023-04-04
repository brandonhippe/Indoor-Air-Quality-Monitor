#include "sps30.h"


#define measurementIx 1 // For PM2.5 Mass Concentration
//#define measurementIx 6 // For PM2.5 Number Concentration


void sps30_sleep() {
  Wire.beginTransmission(ADDR);
  Wire.write(start_sleep);
  Wire.endTransmission();
}


// Non-blocking setup function
void sps30_setup(uint64_t currTime_ms) {
  uint32_t startTime = millis();
  // Put sensor into sleep
  sps30_sleep();

  // Schedule measurement
  sps30_scheduledFunc = sps30_startMeasurement;
}


// Blocking measurement function
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


// Non-blocking measurement functions
void sps30_startMeasurement(uint64_t currTime_ms) {
  uint32_t startTime = millis();
  lastMeasurement = currTime_ms;
  
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

  // Schedule 16 second check
  sps30_scheduledFunc = sps30_highConcen_check;
  sps30_time_ms = currTime_ms + 16000 + startTime - millis();
}

void sps30_highConcen_check(uint64_t currTime_ms) {
  uint32_t startTime = millis();
  
  // Read and check if PM2.5 number concentration > 300/cm^3  
  uint8_t response[30];
  uint16_t check;
  
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

    // Obtain PM2.5 number concentration(s)
    memcpy(&check, &response[18], 2); 

    break;
  }

  // PM2.5 number concentration high, measurement valid
  if (check >= 300) {
    memcpy(&sps30_pm2p5, &response[measurementIx * 3], 2);

    // Put to sleep and schedule next measurement
    sps30_sleep();
    
    sps30_scheduledFunc = sps30_startMeasurement;
    sps30_time_ms = lastMeasurement + sps30_period_ms;
  } else {
    // Wait 14 seconds for data to become valid
    sps30_scheduledFunc = sps30_finalMeasurement;
    sps30_time_ms = currTime_ms + 14000 + startTime - millis();
  }
}


void sps30_finalMeasurement(uint64_t currTime_ms) {
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

    // Obtain measurement
    memcpy(&sps30_pm2p5, &response[measurementIx * 3], 2);

    // Put to sleep and schedule next measurement
    sps30_sleep();
    
    sps30_scheduledFunc = sps30_startMeasurement;
    sps30_time_ms = lastMeasurement + sps30_period_ms;
    break;
  }
}
