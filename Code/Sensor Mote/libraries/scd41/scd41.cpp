#include "scd41.h"


#define SLEEP_LOGIC true
const uint8_t get_serial_number[2] = {0x36, 0x82}, get_data_ready_status[2] = {0xe4, 0xb8};


// High performance
const uint8_t start_periodic_measurement[2] = {0x21, 0xb1}, read_measurement[2] = {0xec, 0x05}, stop_periodic_measurement[2] = {0x3f, 0x86};

// Low power
const uint8_t start_low_power_periodic_measurement[2] = {0x21, 0xac};

// Single Shot
const uint8_t measure_single_shot[2] = {0x21, 0x9d}, set_automatic_self_calibration_initial_period[2] = {0x24, 0x45}, set_automatic_self_calibration_standard_period[2] = {0x24, 0x4e};

// Power-cycled single shot
const uint8_t power_down[2] = {0x36, 0xe0}, wakeup[2] = {0x36, 0xf6};


SCD41::SCD41() {
	max_clock = 400000;
	// period_ms = 0xFFFFFFFFFFFFFFFF;
	period_ms = 60000;
	measurement_ready = false;
	measurement_started = false;
}


boolean SCD41::begin(boolean _debug, int mode, int transistor_sleep) {
	uint32_t startTime = millis();
	debug = _debug;
    MODE = mode;

	transistor_pin = transistor_sleep;
    checks = 0;

	// Wakeup sensor
	transistor_pin == 0 ? scd41_wakeup_I2C() : scd41_wakeup_transistor();
  
	// Initialize the sensor
	if (debug) Serial.println("SCD41: Initializing");
	Wire.beginTransmission(ADDR);
	Wire.write(&stop_periodic_measurement[0], 2);
	int sensed = Wire.endTransmission();
    sleep(500);

	if (debug) Serial.println("SCD41: Wakeup sent");

	if (sensed == 0) {
		scheduledFunc = START_MEASUREMENT;
		if (debug) Serial.println("SCD41: Finished Initialization");
		return true;
	} else {
		time_ms = 0xFFFFFFFFFFFFFFFF;
		if (transistor_pin != 0) digitalWrite(transistor_pin, LOW);
		
		return false;
	}
}


void SCD41::startNextFunc(uint64_t currTime_ms) {
    uint32_t startTime = millis();

	switch (scheduledFunc) {
        case GET_CO2:
            getCO2(currTime_ms);
            break;
        case START_MEASUREMENT:
            measurement_ready = false;

            switch (MODE) {
                case HP_MODE:
                    if (debug) Serial.println("SCD41: HP_MODE Starting Measurement");
                    Wire.beginTransmission(ADDR);
                    Wire.write(&start_periodic_measurement[0], 2);
                    Wire.endTransmission();

                    scheduledFunc = GET_CO2;
                    time_ms = currTime_ms + 5000 + millis() - startTime;
                    break;
                case LP_MODE:
                    if (debug) Serial.println("SCD41: LP_MODE Starting Measurement");
                    Wire.beginTransmission(ADDR);
                    Wire.write(&start_low_power_periodic_measurement[0], 2);
                    Wire.endTransmission();

                    scheduledFunc = GET_CO2;
                    time_ms = currTime_ms + 5000 + millis() - startTime;
                    break;
                case PC_MODE:
                    if (checks == 0) {
                        if (debug) Serial.println("SCD41: Waking up sensor");
                        (transistor_pin == 0 && checks == 0) ? scd41_wakeup_I2C() : scd41_wakeup_transistor();
                    }
                case SS_MODE:
                    if (debug) Serial.println("SCD41: Performing single shot measurment");

                    Wire.beginTransmission(ADDR);
                    Wire.write(&measure_single_shot[0], 2);
                    Wire.endTransmission();

                    scheduledFunc = GET_CO2;
                    time_ms = currTime_ms + 5000 + millis() - startTime;
                    break;
            }
            break;
	}
}


void SCD41::scd41_sleep_transistor() {
	if (!measurement_started) digitalWrite(transistor_pin, SLEEP_LOGIC);

	sleep(5);
}


void SCD41::scd41_sleep_I2C() {
	Wire.beginTransmission(ADDR);
    Wire.write(&power_down[0], 2);
    Wire.endTransmission();
	sleep(1);
}


void SCD41::scd41_wakeup_I2C() {
    Wire.beginTransmission(ADDR);
    Wire.write(&wakeup[0], 2);
    Wire.endTransmission();
	sleep(30);
}


void SCD41::scd41_wakeup_transistor() {
	digitalWrite(transistor_pin, !SLEEP_LOGIC);
	sleep(5);
}


void SCD41::getCO2(uint64_t currTime_ms) {
	uint32_t startTime = millis();
	measurement_ready = false;

    if (checks == 0) {
        lastMeasurement = currTime_ms;
    }

    uint8_t response[9];

    if (debug) Serial.println("SCD41: Checking Data Ready");
    Wire.beginTransmission(ADDR);
    Wire.write(&get_data_ready_status[0], 2);
    Wire.endTransmission();
    sleep(1);

    Wire.requestFrom(ADDR, 3);
    // Serial.println(Wire.available());
    // if (Wire.available() != 3) {
    //     if (debug) Serial.println("Waiting on Data");
    //     while (Wire.available() != 3);
    // }

    for (int i = 0; i < 3; i++) {
        response[i] = Wire.read();
    }

    uint16_t dataReady = (response[0] << 8) + response[1];
    // Serial.println(dataReady, HEX);

    if (check_crc(&response[0]) == 0 || dataReady & 0x07ff == 0) {
        // Data not ready, schedule new check
        if (debug) Serial.println("SCD41: Data Not Ready");
        time_ms = currTime_ms + 1000 + millis() - startTime;
        return;
    }

    if (debug) Serial.println("SCD41: Reading Measurement");
    Wire.beginTransmission(ADDR);
    Wire.write(&read_measurement[0], 2);
    Wire.endTransmission();
    sleep(10);

    Wire.requestFrom(ADDR, 9);
    // Serial.println(Wire.available());
    // if (Wire.available() != 9) {
    //     if (debug) Serial.println("Waiting on Data");
    //     while (Wire.available() != 9);
    // }

    for (int i = 0; i < 9; i++) {
        response[i] = Wire.read();
        // Serial.println(response[i], HEX);
    }

    // This line will fail
    // if (debug) {
    //     Serial.println(calculate_crc(&response[0]), HEX);
    //     Serial.println(response[2], HEX);
    //     Serial.println(calculate_crc(&response[3]), HEX);
    //     Serial.println(response[5], HEX);
    //     Serial.println(calculate_crc(&response[6]), HEX);
    //     Serial.println(response[8], HEX);
    // }

    if (check_crc(&response[0]) == 0 || check_crc(&response[3]) == 0 || check_crc(&response[6]) == 0) {
        // Data not ready, schedule new check
        if (debug) Serial.println("SCD41: CRC Check Failed");
        time_ms = currTime_ms + 1000 + millis() - startTime;
        return;
    }

    co2 = ((response[0] << 8) + response[1]) / 1.0;
    temp = -45 + 175 * ((response[3] << 8) + response[4]) / 65535.0;
    rh = 100 * ((response[6] << 8) + response[7]) / 65535.0;

    // Schedule new measurement
    switch (MODE) {
        case HP_MODE:
            if (debug) Serial.println("SCD41: HP_MODE Scheduling new measurement");
            measurement_ready = true;
            time_ms = currTime_ms + 5000 + millis() - startTime;
            break;
        case LP_MODE:
            if (debug) Serial.println("SCD41: LP_MODE Scheduling new measurement");
            measurement_ready = true;
            time_ms = currTime_ms + 30000 + millis() - startTime;;
            break;
        case SS_MODE:
        case PC_MODE:
            checks++;
            checks %= 3;

            scheduledFunc = START_MEASUREMENT;

            if (checks == 0) {
                if (debug) Serial.println("SCD41: Scheduling new signle shot");
                measurement_ready = true;
                time_ms = lastMeasurement + period_ms;
                if (MODE == PC_MODE) {
                    transistor_pin == 0 ? scd41_sleep_I2C() : scd41_sleep_transistor();
                }
            } else {
                time_ms = currTime_ms + 200 + millis() - startTime;
            }

            break;
    }
}

void SCD41::setPeriod(uint64_t newPeriod_ms) {
    period_ms = newPeriod_ms;

    if (MODE == SS_MODE) {
        uint16_t initialCalibration, calibrationPeriod;

        // Calculate periods until initial calibration. Should take place after 2 days
        initialCalibration = 172800000 / period_ms;   // ms in 2 days / period in ms
        initialCalibration = ((initialCalibration & 0xFF) << 8) + (initialCalibration >> 8);
        Wire.beginTransmission(ADDR);
        Wire.write(&set_automatic_self_calibration_initial_period[0], 2);
        Wire.write((uint8_t*)&initialCalibration, 2);
        Wire.write(calculate_crc(&initialCalibration));
        Wire.endTransmission();
        sleep(1);

        // Calculate periods between calibrations. Should be 7 days
        calibrationPeriod = 604800000 / period_ms;  // ms in 7 days / period in ms
        calibrationPeriod = ((calibrationPeriod & 0xFF) << 8) + (calibrationPeriod >> 8);
        Wire.beginTransmission(ADDR);
        Wire.write(&set_automatic_self_calibration_standard_period[0], 2);
        Wire.write((uint8_t*)&calibrationPeriod, 2);
        Wire.write(calculate_crc(&calibrationPeriod));
        Wire.endTransmission();
        sleep(1);
    }
}
