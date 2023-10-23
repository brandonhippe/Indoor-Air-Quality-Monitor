#include "sgp30.h"


#define SLEEP_LOGIC true
const uint8_t iaq_init[2] = {0x20, 0x03}, measure_iaq[2] = {0x20, 0x08}, get_iaq_baseline[2] = {0x20, 0x15}, set_iaq_baseline[2] = {0x20, 0x1e};
const bool avg = false;


SGP30::SGP30() {
	max_clock = 400000;
	period_ms = 0xFFFFFFFFFFFFFFFF;
	// period_ms = 32000;
	measurement_ready = false;
	measurement_started = false;
}


boolean SGP30::begin(boolean _debug, int transistor_sleep) {
	return begin(_debug, transistor_sleep, 0, 0);
}


boolean SGP30::begin(boolean _debug, int transistor_sleep, int co2Baseline, int tvocBaseline) {
	uint32_t startTime = millis();
	debug = _debug;

	transistor_pin = transistor_sleep;

	baseline_CO2 = ((co2Baseline & 0xFF) << 8) + ((co2Baseline & 0xFF00) >> 8);
	baselineTVOC = ((tvocBaseline & 0xFF) << 8) + ((tvocBaseline & 0xFF00) >> 8);

	// Wakeup sensor
	transistor_pin == 0 ? sgp30_wakeup_I2C() : sgp30_wakeup_transistor();
  
	// Initialize the sensor
	if (debug) Serial.println("SGP30: Initializing");
	Wire.beginTransmission(ADDR);
	Wire.write(&iaq_init[0], 2);
	int sensed = Wire.endTransmission();
	if (debug) Serial.println("SGP30: Wakeup sent");

	if (sensed == 0) {
		// Perform calibration
		// calibration(millis());
		// if (baseline_CO2 == 0 && baselineTVOC == 0) {
		// 	scheduledFunc = GETCO2;
		// } else {
		// 	scheduledFunc = SET_CALIBRATION;
		// }
		scheduledFunc = GETCO2;

		if (debug) Serial.println("SGP30: Finished Initialization");
		return true;
	} else {
		time_ms = 0xFFFFFFFFFFFFFFFF;
		if (transistor_pin != 0) digitalWrite(transistor_pin, LOW);
		
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
	if (!measurement_started) digitalWrite(transistor_pin, SLEEP_LOGIC);
	sleep(5);
}


void SGP30::sgp30_sleep_I2C() {
	// This performs a general call reset. It is possible that this resets other I2C devices on the bus
	if (!measurement_started) {
		Wire.beginTransmission(0x00);
		Wire.write(0x06);
		Wire.endTransmission();
	}
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
	checks = 0;

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
		// Restart request if either CRC check fails
		if (check_crc(&response[0]) == 0) continue;
		if (check_crc(&response[3]) == 0) continue;

		// Obtain CO2 and TVOC values
		co2 = (response[0] << 8) + response[1];
		tvoc = (response[3] << 8) + response[4];

		if (debug) {
			Serial.println(co2);
			Serial.println(tvoc);
		}

		// if (!(co2 == 400 && tvoc == 0)) {
		if (checks == 60) {
			checks = 0;
			break;
		}

		checks++;
		sleep(1000);
	}

	if (debug) Serial.println("SGP30: Obtaining calibration values");

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

		if (debug) {
			Serial.print("SGP30: Calibration Values: Baseline CO2: ");
  			Serial.print(baseline_CO2);
  			Serial.print(", Baseline TVOC: ");
  			Serial.println(baselineTVOC);
		}

		break;
	}

	// Put sensor to sleep
	if (debug) Serial.println("SGP30: Sleeping");
	if (transistor_pin == 0) sgp30_sleep_I2C();

	// Schedule measurement request
	scheduledFunc = GETCO2;
	time_ms = millis();
}


void SGP30::setCalibration(uint64_t currTime_ms) {
	uint32_t startTime = millis();
	measurement_ready = false;

	checks = 0;
	lastMeasurement = currTime_ms;
	measurement_started = true;
  
	// Turn on sensor
	if (debug) Serial.println("SGP30: Waking up");
	transistor_pin == 0 ? sgp30_wakeup_I2C() : sgp30_wakeup_transistor();

	if (debug) Serial.println("SGP30: Initializing");
	Wire.beginTransmission(ADDR);
	Wire.write(&iaq_init[0], 2);
	Wire.endTransmission();
	sleep(20);

	// if (debug) {
	// 	Serial.print("SGP30: Calibration Values: Baseline CO2: ");
	// 	Serial.print(baseline_CO2);
	// 	Serial.print(", Baseline TVOC: ");
	// 	Serial.println(baselineTVOC);
	// }

	// Write calibration values
	if (debug) Serial.println("SGP30: Writing Calibration");
	Wire.beginTransmission(ADDR);
	Wire.write(&set_iaq_baseline[0], 2);
	Wire.write((uint8_t *) &baselineTVOC, 2);
	Wire.write(calculate_crc(&baselineTVOC));
	Wire.write((uint8_t *) &baseline_CO2, 2);
	Wire.write(calculate_crc(&baseline_CO2));
	Wire.endTransmission();
	sleep(10);

	scheduledFunc = GETCO2;
	time_ms = currTime_ms + 100 + millis() - startTime;
}


void SGP30::getCO2(uint64_t currTime_ms) {
	uint32_t startTime = millis();
	measurement_ready = false;

	int measured_co2, measured_tvoc;


	if (debug) Serial.println("SGP30: Starting measurement");
	// Wakeup sensor
	transistor_pin == 0 ? sgp30_wakeup_I2C() : sgp30_wakeup_transistor();

	if (!measurement_started) {
		lastMeasurement = currTime_ms;
		measurement_started = true;

		if (debug) Serial.println("SGP30: Initializing");
		Wire.beginTransmission(ADDR);
		Wire.write(&iaq_init[0], 2);
		Wire.endTransmission();
		delay(20);

		if (baseline_CO2 != 0 && baselineTVOC != 0) {
			// Write calibration values
			if (debug) Serial.println("SGP30: Writing Calibration");

			Wire.beginTransmission(ADDR);
			Wire.write(&set_iaq_baseline[0], 2);
			Wire.write((uint8_t *) &baselineTVOC, 2);
			Wire.write(calculate_crc(&baselineTVOC));
			Wire.write((uint8_t *) &baseline_CO2, 2);
			Wire.write(calculate_crc(&baseline_CO2));
			Wire.endTransmission();

			delay(10);
		}
	}

	if (debug) Serial.println(checks);

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
		measured_co2 = (response[0] << 8) + response[1];
		measured_tvoc = (response[3] << 8) + response[4];

		if (checks >= 15) {
			if (debug) Serial.println(checks - 15);
			co2_readings[checks - 15] = measured_co2;
			tvoc_readings[checks - 15] = measured_tvoc;
		}

		if (debug) {
			Serial.println(measured_co2);
			Serial.println(measured_tvoc);
		}
		
		// Schedule next measurement
		if (debug) Serial.println("SGP30: Scheduling new measurement");
		if (checks < 30) {
			checks++;
			scheduledFunc = GETCO2;
			time_ms = currTime_ms + 1200 + millis() - startTime;

			return;
		} else {
			co2 = 0;
			for (int i = 0; i < 15; i++) {
				if (avg) {
					co2 += co2_readings[i];
				} else if (co2_readings[i] > co2) {
					co2 = co2_readings[i];
				}
			}

			if (avg) co2 = co2 / 15.0;
			if (debug) Serial.println(co2);
			
			checks = 0;
			measurement_ready = true;
			measurement_started = false;

			// if (baseline_CO2 == 0 && baselineTVOC == 0) {
			// 	scheduledFunc = GETCO2;
			// } else {
			// 	scheduledFunc = SET_CALIBRATION;
			// }
			scheduledFunc = GETCO2;

			time_ms = lastMeasurement + period_ms;
		}

		break;
	}

	// Put sensor to sleep
	if (debug) Serial.println("SGP30: Sleeping");
	transistor_pin == 0 ? sgp30_sleep_I2C() : sgp30_sleep_transistor();
}
