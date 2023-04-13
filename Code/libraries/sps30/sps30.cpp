#include "sps30.h"


#ifdef DEBUG
boolean sps_debug = true;
#else
boolean sps_debug = false;
#endif


SPS30::SPS30() {
  max_clock = 100000;
	period_ms = 60000;
	start_measurement = 0x0010;
	stop_measurement = 0x0014;
	data_ready = 0x0202;
	read_measured = 0x0300;
	start_sleep = 0x1001;
	wakeup = 0x1103;
  measurement_ready = false;
	measurementIx = 1; // 1 for PM2.5 mass concentration, 6 for PM2.5 number concentration
}


boolean SPS30::begin(uint64_t currTime_ms) {
  uint32_t startTime = millis();
  // Put sensor into sleep
  sps30_sleep();

  // Schedule measurement
  scheduledFunc = START_MEASUREMENT;
  time_ms = currTime_ms + 100 + startTime - millis();
}


void SPS30::startNextFunc(uint64_t currTime_ms) {
	switch (scheduledFunc) {
		case START_MEASUREMENT:
			startMeasurement(currTime_ms);
			break;
		case CONCENTRATION_CHECK:
			highConcen_check(currTime_ms);
			break;
		case FINISH_MEASUREMENT:
			finalMeasurement(currTime_ms);
			break;
	}
}


void SPS30::sps30_sleep() {
  Wire.beginTransmission(ADDR);
  Wire.write(start_sleep);
  Wire.endTransmission();
}


void SPS30::startMeasurement(uint64_t currTime_ms) {
  uint32_t startTime = millis();
  lastMeasurement = currTime_ms;
  measurement_ready = false;
  
  if (sps_debug) {
    Serial.println("Starting measurement");
  }

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
  Wire.write(&init_vals[0], 2); // Set up for 16-bit integer values
  Wire.write(calculate_crc(&init_vals[0]));
  Wire.endTransmission();
  delay(20);

  // Schedule 16 second check
  scheduledFunc = CONCENTRATION_CHECK;
  time_ms = currTime_ms + 16000 + startTime - millis();
}

void SPS30::highConcen_check(uint64_t currTime_ms) {
  uint32_t startTime = millis();
  
  if (sps_debug) {
    Serial.println("Starting high concentration check");
  }

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
    measurement_ready = true;
    memcpy(&pm2p5, &response[measurementIx * 3], 2);

    // Put to sleep and schedule next measurement
    sps30_sleep();
    
    scheduledFunc = START_MEASUREMENT;
    time_ms = lastMeasurement + period_ms;
  } else {
    // Wait 14 seconds for data to become valid
    scheduledFunc = FINISH_MEASUREMENT;
    time_ms = currTime_ms + 14000 + startTime - millis();
  }
}


void SPS30::finalMeasurement(uint64_t currTime_ms) {
  uint8_t response[30];
  measurement_ready = true;

  if (sps_debug) {
    Serial.println("Starting final measurement");
  }
  
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
    memcpy(&pm2p5, &response[measurementIx * 3], 2);

    // Put to sleep and schedule next measurement
    sps30_sleep();
    
    scheduledFunc = START_MEASUREMENT;
    time_ms = lastMeasurement + period_ms;
    break;
  }
}
