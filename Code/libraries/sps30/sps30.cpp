#include "sps30.h"


static const uint8_t start_measurement[2] = {0x00, 0x10}, stop_measurement[2] = {0x01, 0x04}, data_ready[2] = {0x02, 0x02}, read_measured[2] = {0x03, 0x00}, start_sleep[2] = {0x10, 0x01}, wakeup[2] = {0x11, 0x03};
static uint8_t init_vals[2] = {0x05, 0x00};


SPS30::SPS30() {
	max_clock = 100000;
	period_ms = 60000;
	measurement_ready = false;
}


boolean SPS30::begin(int measurement, uint64_t currTime_ms, boolean _debug) {
	uint32_t startTime = millis();
	debug = _debug;
	measurementIx = measurement;

	// Schedule measurement
	scheduledFunc = START_MEASUREMENT;
	time_ms = currTime_ms + 100 + millis() - startTime;
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
	Wire.write((uint8_t*) &stop_measurement[0], 2);
	Wire.endTransmission();
	delay(20);
  
	Wire.beginTransmission(ADDR);
	Wire.write((uint8_t*) &start_sleep[0], 2);
	Wire.endTransmission();
	delay(5);
}


void SPS30::startMeasurement(uint64_t currTime_ms) {
	uint32_t startTime = millis();
	lastMeasurement = currTime_ms;
	measurement_ready = false;
  
	if (debug) {
		Serial.println("SPS30: Starting measurement");
	}

	// Turn on sensor
	Wire.beginTransmission(ADDR);
	Wire.write((uint8_t*) &wakeup[0], 2);
	Wire.endTransmission();
	delay(5);
  
	// Send wakeup again to finish wakeup command
	Wire.beginTransmission(ADDR);
	Wire.write((uint8_t*) &wakeup[0], 2);
	Wire.endTransmission();
	delay(5);

	// Put into measurement mode
	Wire.beginTransmission(ADDR);
	Wire.write((uint8_t*) &start_measurement[0], 2);
	Wire.write(&init_vals[0], 2); // Set up for 16-bit integer values
	Wire.write(calculate_crc(&init_vals[0]));
	Wire.endTransmission();

	// Schedule 16 second check
	scheduledFunc = CONCENTRATION_CHECK;
	time_ms = currTime_ms + 16000 + millis() - startTime;
	
	// // Schedule Measurement
	// scheduledFunc = FINISH_MEASUREMENT;
	// time_ms = currTime_ms + 31000 + millis() - startTime;
}

void SPS30::highConcen_check(uint64_t currTime_ms) {
	uint32_t startTime = millis();
  
	if (debug) {
		Serial.println("SPS30: Starting high concentration check");
	}

	// Read and check if PM2.5 number concentration > 300/cm^3  
	uint8_t response[30];
	uint16_t check;
  
	while (1) {
		Wire.beginTransmission(ADDR);
		Wire.write((uint8_t*) &read_measured[0], 2);
		Wire.endTransmission();
	
		delay(20);
		Wire.requestFrom(ADDR, 30);

		while (Wire.available() < 30);
	
		for (int i = 0; i < 30; i++) {
			response[i] = Wire.read();
		}

		// Restart request if any CRC check fails
		int i;
		for (i = 0; i < 30; i += 3) {
			if (check_crc(&response[i]) == 0) break;
		}

		if (i != 30) continue;

		// Obtain PM2.5 number concentration(s)
		check = (response[18] << 8) + response[19];

		break;
	}

	// PM2.5 number concentration high, measurement valid
	if (check >= 300) {
		measurement_ready = true;
		pm2p5  = (response[measurementIx * 3] << 8) + response[(measurementIx * 3) + 1];

		// Put to sleep and schedule next measurement
		sps30_sleep();
    
		scheduledFunc = START_MEASUREMENT;
		time_ms = lastMeasurement + period_ms;
	} else {
		// Wait 14 seconds for data to become valid
		scheduledFunc = FINISH_MEASUREMENT;
		time_ms = currTime_ms + 15000 + millis() - startTime;
	}
}


void SPS30::finalMeasurement(uint64_t currTime_ms) {
	uint8_t response[30];
	measurement_ready = true;

	if (debug) {
		Serial.println("SPS30: Starting final measurement");
	}
  
	while (1) {
		Wire.beginTransmission(ADDR);
		Wire.write((uint8_t*) &read_measured[0], 2);
		Wire.endTransmission();
	
		delay(20);
		Wire.requestFrom(ADDR, 30);

		while (Wire.available() < 30);
	
		for (int i = 0; i < 30; i++) {
			response[i] = Wire.read();
		}

		// Restart request if any CRC check fails
		int i;
		for (i = 0; i < 30; i += 3) {
			if (check_crc(&response[i]) == 0) break;
		}

		if (i != 30) continue;

		// Obtain measurement
		pm2p5  = (response[measurementIx * 3] << 8) + response[(measurementIx * 3) + 1];

		// Put to sleep and schedule next measurement
		sps30_sleep();
    
		scheduledFunc = START_MEASUREMENT;
		time_ms = lastMeasurement + period_ms;
		break;
	}
}
