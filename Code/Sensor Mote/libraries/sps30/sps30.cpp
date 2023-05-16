#include "sps30.h"


#define SLEEP_LOGIC true
static const uint8_t start_measurement[2] = {0x00, 0x10}, stop_measurement[2] = {0x01, 0x04}, data_ready[2] = {0x02, 0x02}, read_measured[2] = {0x03, 0x00}, start_sleep[2] = {0x10, 0x01}, wakeup[2] = {0x11, 0x03};
static uint8_t init_vals[2];


SPS30::SPS30() {
	max_clock = 100000;
	period_ms = 4680000;
	measurement_ready = false;
}


boolean SPS30::begin(int measurement, boolean _fp, boolean _debug, int transistor_sleep) {
	uint32_t startTime = millis();
	debug = _debug;
	measurementIx = measurement;
	fp = _fp;

	transistor_pin = transistor_sleep;

	if (fp) {
		init_vals[0] = 0x03;
		bytes_needed = 60;
	} else {
		init_vals[0] = 0x05;
		bytes_needed = 30;
	}

	init_vals[1] = 0x00;

	transistor_pin == 0 ? sps30_wakeup_I2C() : sps30_wakeup_transistor();

	bool db = debug;
	debug = false;
	Wire.beginTransmission(ADDR);
	Wire.write((uint8_t*) &wakeup[0], 2);
	boolean sensed = Wire.endTransmission();
	sleep(5);
	debug = db;

	// Schedule measurement
	scheduledFunc = START_MEASUREMENT;
	time_ms = millis();


	if (sensed == 0) {
		return true;
	} else {
		time_ms = 0xFFFFFFFFFFFFFFFF;
		transistor_pin == 0 ? sps30_sleep_I2C() : sps30_sleep_transistor();
		return false;
	}
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


void SPS30::sps30_sleep_I2C() {
	if (debug) Serial.println("SPS30: Stopping measurement");
	Wire.beginTransmission(ADDR);
	Wire.write((uint8_t*) &stop_measurement[0], 2);
	Wire.endTransmission();
	sleep(20);
	
	if (debug) Serial.println("SPS30: Sleeping");
	Wire.beginTransmission(ADDR);
	Wire.write((uint8_t*) &start_sleep[0], 2);
	Wire.endTransmission();
	sleep(5);
}


void SPS30::sps30_sleep_transistor() {
	digitalWrite(transistor_pin, SLEEP_LOGIC);
	sleep(5);
}


void SPS30::sps30_wakeup_I2C() {
	// Turn on sensor
	if (debug) Serial.println("SPS30: Initializing");
	Wire.beginTransmission(ADDR);
	Wire.write((uint8_t*) &wakeup[0], 2);
	Wire.endTransmission();
	sleep(5);
  
	// Send wakeup again to finish wakeup command
	Wire.beginTransmission(ADDR);
	Wire.write((uint8_t*) &wakeup[0], 2);
	int sensed = Wire.endTransmission();
	sleep(5);
}


void SPS30::sps30_wakeup_transistor() {
	digitalWrite(transistor_pin, !SLEEP_LOGIC);
	sleep(5);
}


void SPS30::startMeasurement(uint64_t currTime_ms) {
	uint32_t startTime = millis();
	lastMeasurement = currTime_ms;
	measurement_ready = false;
  
	// Turn on sensor
	if (debug) Serial.println("SPS30: Waking up");
	transistor_pin == 0 ? sps30_wakeup_I2C() : sps30_wakeup_transistor();

	// Put into measurement mode
	if (debug) Serial.println("SPS30: Starting measurement");
	Wire.beginTransmission(ADDR);
	Wire.write((uint8_t*) &start_measurement[0], 2);
	Wire.write(&init_vals[0], 2);
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
  
	if (debug) Serial.println("SPS30: Starting high concentration check");

	// Read and check if PM2.5 number concentration > 300/cm^3  
	uint8_t response[bytes_needed];
  
	while (1) {
		if (debug) Serial.println("SPS30: Sending read request");
		Wire.beginTransmission(ADDR);
		Wire.write((uint8_t*) &read_measured[0], 2);
		Wire.endTransmission();
	
		sleep(20);
		Wire.requestFrom(ADDR, bytes_needed);

		while (Wire.available() < bytes_needed);

		if (debug) Serial.println("SPS30: Reading data");
		for (int i = 0; i < bytes_needed; i++) {
			response[i] = Wire.read();
		}

		// Restart request if any CRC check fails
		if (debug) Serial.println("SPS30: CRC Checks");
		int i;
		for (i = 0; i < bytes_needed; i += 3) {
			if (check_crc(&response[i]) == 0) break;
		}

		if (i != bytes_needed) {
			if (debug) {
				Serial.println("CRC CHECK FAILED!");
				Serial.println(i);
			}

			continue;
		}

		// Obtain PM2.5 number concentration(s)
		if (debug) Serial.println("SPS30: Reading measured values");
		if (fp) {
			uint8_t float_bytes[4] = {response[40], response[39], response[37], response[36]};
			pm2p5_float = *(float*)(&float_bytes[0]);
		} else {
			pm2p5_int = (response[18] << 8) + response[19];
		}

		break;
	}

	if (debug) {
	}

	// PM2.5 number concentration high, measurement valid
	if (debug) Serial.println("SPS30: Scheduling new measurement");
	if ((!fp && pm2p5_int >= 300) || (fp && pm2p5_float >= 300)) {
		measurement_ready = true;
		if (fp) {
			uint8_t float_bytes[4] = {response[(measurementIx * 6) + 4], response[(measurementIx * 6) + 3], response[(measurementIx * 6) + 1], response[(measurementIx * 6)]};
			pm2p5_float = *(float*)(&float_bytes[0]);
		} else {
			pm2p5_int = (response[measurementIx * 3] << 8) + response[(measurementIx * 3) + 1];
		}

		// Put to sleep and schedule next measurement
		transistor_pin == 0 ? sps30_sleep_I2C() : sps30_sleep_transistor();
    
		scheduledFunc = START_MEASUREMENT;
		time_ms = lastMeasurement + period_ms;
	} else {
		// Wait 14 seconds for data to become valid
		scheduledFunc = FINISH_MEASUREMENT;
		time_ms = currTime_ms + 15000 + millis() - startTime;
	}
}


void SPS30::finalMeasurement(uint64_t currTime_ms) {
	uint8_t response[bytes_needed];
	measurement_ready = true;

	if (debug) Serial.println("SPS30: Starting final measurement");
  
	while (1) {
		if (debug) Serial.println("SPS30: Sending read request");
		Wire.beginTransmission(ADDR);
		Wire.write((uint8_t*) &read_measured[0], 2);
		Wire.endTransmission();
	
		sleep(20);
		Wire.requestFrom(ADDR, bytes_needed);

		while (Wire.available() < bytes_needed);

		if (debug) Serial.println("SPS30: Reading data");
		for (int i = 0; i < bytes_needed; i++) {
			response[i] = Wire.read();
		}

		// Restart request if any CRC check fails
		if (debug) Serial.println("SPS30: CRC Checks");
		int i;
		for (i = 0; i < bytes_needed; i += 3) {
			if (check_crc(&response[i]) == 0) break;
		}

		if (i != bytes_needed) continue;

		// Obtain measurement
		if (debug) Serial.println("SPS30: Reading measured values");
		if (fp) {
			uint8_t float_bytes[4] = {response[(measurementIx * 4)], response[(measurementIx * 6) + 3], response[(measurementIx * 6) + 1], response[(measurementIx * 6)]};
			pm2p5_float = *(float*)(&float_bytes[0]);
		} else {
			pm2p5_int = (response[measurementIx * 3] << 8) + response[(measurementIx * 3) + 1];
		}

		// Put to sleep and schedule next measurement
		if (debug) Serial.println("SPS30: Scheduling new measurement");
		transistor_pin == 0 ? sps30_sleep_I2C() : sps30_sleep_transistor();
		scheduledFunc = START_MEASUREMENT;
		time_ms = lastMeasurement + period_ms;
		break;
	}
}
