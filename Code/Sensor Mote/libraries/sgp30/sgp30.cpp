#include "sgp30.h"


#define SLEEP_LOGIC true
const uint8_t iaq_init[2] = {0x20, 0x03}, measure_iaq[2] = {0x20, 0x08}, get_iaq_baseline[2] = {0x20, 0x15}, set_iaq_baseline[2] = {0x20, 0x1e};


SGP30::SGP30() {
	max_clock = 400000;
	period_ms = 1320000;
	measurement_ready = false;
	measurementStarted = false;
}


boolean SGP30::begin(boolean _debug, int transistor_sleep) {
	uint32_t startTime = millis();
	debug = _debug;

	transistor_pin = transistor_sleep;

	// Wakeup sensor
	transistor_pin == 0 ? sgp30_wakeup_I2C() : sgp30_wakeup_transistor();
  
	// Initialize the sensor
	if (debug) Serial.println("SGP30: Initializing");
	Wire.beginTransmission(ADDR);
	Wire.write(&iaq_init[0], 2);
	int sensed = Wire.endTransmission();

	if (sensed == 0) {
		// Perform calibration
		calibration(millis());
		time_ms = millis();
		return true;
	} else {
		time_ms = 0xFFFFFFFFFFFFFFFF;
		transistor_pin == 0 ? sgp30_sleep_I2C() : sgp30_sleep_transistor();
		return false;
	}
}


void SGP30::startNextFunc(uint64_t currTime_ms) {
	switch (scheduledFunc) {
		case CALIBRATION:
			calibration(currTime_ms);
			break;
		case SET_CALIBRATION:
			setCalibration(currTime_ms);
			break;
		case GETCO2:
			getCO2(currTime_ms);
			break;
	}
}


void SGP30::sgp30_sleep_transistor() {
	digitalWrite(transistor_pin, SLEEP_LOGIC);
	sleep(5);
}


void SGP30::sgp30_sleep_I2C() {
	// This performs a general call reset. It is possible that this resets other I2C devices on the bus
	if (debug) Serial.println("SGP30: Sleeping");
	Wire.beginTransmission(0x00);
	Wire.write(0x06);
	Wire.endTransmission();
	sleep(5);
}


void SGP30::sgp30_wakeup_I2C() {
	sleep(5);
}


void SGP30::sgp30_wakeup_transistor() {
	digitalWrite(transistor_pin, !SLEEP_LOGIC);
	sleep(5);
}



void SGP30::calibration(uint64_t currTime_ms) {
	uint32_t startTime = millis();
	lastMeasurement = currTime_ms;
	measurement_ready = false;

	if (debug) Serial.println("SGP30: Performing Calibration");
  
	// Get measurement data
	while (1) {
		if (debug) Serial.println("SGP30: Sending Measure Request");

		Wire.beginTransmission(ADDR);
		Wire.write(&measure_iaq[0], 2);
		Wire.endTransmission();
		
		sleep(12);
		Wire.requestFrom(ADDR, 6);
		
		while (Wire.available() < 6);
		
		uint8_t response[6];
		for (int i = 0; i < 6; i++) {
			response[i] = Wire.read();
		}

		// Check if new calibration measurement is needed, schedule and exit if needed
		if (((response[0] << 8) + response[1]) == 400 && ((response[3] << 8) + response[4]) == 0) {
			// time_ms = currTime_ms + 1000 + millis() - startTime;
			if (debug) Serial.println("SGP30: Not calibrated, waiting 1 second");
			sleep(1000);
		} else {
			break;
		}
	}

	if (debug) {
		Serial.println("SGP30: Obtaining calibration values");
	}

	// Obtain the measured baseline values to skip calibration process later
	while (1) {
		Wire.beginTransmission(ADDR);
		Wire.write(&get_iaq_baseline[0], 2);
		Wire.endTransmission();
    
		sleep(10);
		Wire.requestFrom(ADDR, 6);

		// Restart request if less than 6 bytes available
		if (Wire.available() < 6) continue;
		uint8_t response[6];
		for (int i = 0; i < 6; i++) {
			response[i] = Wire.read();
		}

		// Restart request if either CRC check fails
		if (check_crc(&response[0]) == 0) continue;
		if (check_crc(&response[3]) == 0) continue;

		// Store calibrated baseline values
		baseline_CO2 = (response[0] << 8) + response[1];
		baselineTVOC = (response[3] << 8) + response[2];
		break;
	}

	// Put sensor to sleep
	transistor_pin == 0 ? sgp30_sleep_I2C() : sgp30_sleep_transistor();

	// Schedule measurement request
	scheduledFunc = SET_CALIBRATION;
	time_ms = lastMeasurement + period_ms;
}


void SGP30::setCalibration(uint64_t currTime_ms) {
	uint32_t startTime = millis();
	measurement_ready = false;
	checks = 0;

	if (debug) {
		Serial.println("SGP30: Writing Calibration");
	}
  
	// Turn on sensor
	transistor_pin == 0 ? sgp30_wakeup_I2C() : sgp30_wakeup_transistor();
	Wire.beginTransmission(ADDR);
	Wire.write(&iaq_init[0], 2);
	Wire.endTransmission();
	sleep(10);

	// Write calibration values
	Wire.beginTransmission(ADDR);
	Wire.write(&set_iaq_baseline[0], 2);
	Wire.write((uint8_t *) &baselineTVOC, 2);
	Wire.write(calculate_crc(&baselineTVOC));
	Wire.write((uint8_t *) &baseline_CO2, 2);
	Wire.write(calculate_crc(&baseline_CO2));
	Wire.endTransmission();
	sleep(10);

	scheduledFunc = GETCO2;
	time_ms = currTime_ms + 1000 + millis() - startTime;
}


void SGP30::getCO2(uint64_t currTime_ms) {
	uint32_t startTime = millis();
	measurement_ready = false;

	if (!measurementStarted) {
		lastMeasurement = currTime_ms;
		measurementStarted = true;
	}


	if (debug) Serial.println("SGP30: Starting measurement");

	// Perform measurement
	while (1) {
		if (debug) Serial.println("SGP30: Sending measurement request");

		Wire.beginTransmission(ADDR);
		Wire.write(&measure_iaq[0], 2);
		Wire.endTransmission();
	
		sleep(12);
		Wire.requestFrom(ADDR, 6);
	
		if (Wire.available() < 6) continue;

		if (debug) Serial.println("SGP30: Reading measured values");
		uint8_t response[6];
		for (int i = 0; i < 6; i++) {
			response[i] = Wire.read();
		}

		// Restart request if either CRC check fails
		if (debug) Serial.println("SGP30: CRC Checks");
		if (check_crc(&response[0]) == 0) continue;
		if (check_crc(&response[3]) == 0) continue;
		
		// Obtain CO2 and TVOC values
		co2 = (response[0] << 8) + response[1];
		tvoc = (response[3] << 8) + response[4];
		
		// Schedule next measurement
		if (debug) Serial.println("SGP30: Scheduling new measurement");
		// if (co2 == 400 && tvoc == 0) {
		if (checks < 15) {
			checks++;
			scheduledFunc = GETCO2;
			time_ms = currTime_ms + 1000 + millis() - startTime;

			return;
		} else {

			measurement_ready = true;
			measurementStarted = false;
			scheduledFunc = SET_CALIBRATION;
			// scheduledFunc = GETCO2;
			time_ms = lastMeasurement + period_ms;
		}

		break;
	}

	// Put sensor to sleep
	transistor_pin == 0 ? sgp30_sleep_I2C() : sgp30_sleep_transistor();
}
