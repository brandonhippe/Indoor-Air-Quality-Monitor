#include "CGAnem.h"


const uint8_t dev_id = 0x05, windMSB = 0x07, windLSB = 0x08;


CGAnem::CGAnem() {
	max_clock = 200000;
	period_ms = 30000;
	measurement_ready = false;
}


boolean CGAnem::begin(int sleepPin, boolean _debug) {
	uint32_t startTime = millis();
	debug = _debug;

	sleep_pin = sleepPin;
	digitalWrite(sleep_pin, HIGH);
	pinMode(sleep_pin, OUTPUT);
	
	Wire.beginTransmission(ADDR);
	Wire.write(dev_id);
	if (Wire.endTransmission() != 0) {
		time_ms = 0xFFFFFFFF;
		return false;
	}
	
	// Put sensor to sleep
	digitalWrite(sleep_pin, LOW);	

	// Schedule Measurement
	scheduledFunc = START_MEASUREMENT;
	time_ms = millis();
  
	return true;
}


void CGAnem::startNextFunc(uint64_t currTime_ms) {
	switch (scheduledFunc) {
		case START_MEASUREMENT:
			start_measurement(currTime_ms);
			break;
		case FINISH_MEASUREMENT:
			finish_measurement(currTime_ms);
			break;
	}
}


void CGAnem::start_measurement(uint64_t currTime_ms) {
	uint32_t startTime = millis();
	lastMeasurement = currTime_ms;
	measurement_ready = false;
	
	// Wakeup sensor
	if (debug) {
		Serial.println("CGAnem: Start Measurement");
	}
	
	digitalWrite(sleep_pin, HIGH);
	
	// Schedule measurement finish
	scheduledFunc = FINISH_MEASUREMENT;
	time_ms = currTime_ms + 10000 + millis() - startTime;
}


void CGAnem::finish_measurement(uint64_t currTime_ms) {
	uint32_t startTime = millis();
	measurement_ready = true;
	
	// Get measurement from sensor
	if (debug) {
		Serial.println("CGAnem: Finish Measurement");
	}
	
	Wire.beginTransmission(ADDR);
	Wire.write(windMSB);
	Wire.endTransmission(true);
	Wire.requestFrom(ADDR, 1);
	
	while (Wire.available() == 0);
	wind = (Wire.read()) << 8;
	
	Wire.beginTransmission(ADDR);
	Wire.write(windLSB);
	Wire.endTransmission(true);
	Wire.requestFrom(ADDR, 1);
	
	while (Wire.available() == 0);
	wind += Wire.read();
	
	// Put sensor to sleep
	digitalWrite(sleep_pin, LOW);
	
	// Schedule next measurement
	scheduledFunc = START_MEASUREMENT;
	time_ms = lastMeasurement + period_ms;
}