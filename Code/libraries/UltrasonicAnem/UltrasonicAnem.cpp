#include "UltrasonicAnem.h"


const double a = 0.05, d = 0.05; // a and d in meters
double sin_alpha, cos_alpha;


UltrasonicAnem::UltrasonicAnem() {
	max_clock = 400000;
	period_ms = 60000;
	measurement_ready = false;
	measurementStarted = false;
	tx1 = 11;
	rx1 = 12;
	tx2 = 13;
	rx2 = 14;
	
	double alpha = atan(2 * d / a);
	sin_alpha = sin(alpha);
	cos_alpha = cos(alpha);
}


boolean UltrasonicAnem::begin(int sleepPin, uint64_t currTime_ms, boolean _debug) {
	uint32_t startTime = millis();
	debug = _debug;
	sleep_pin = sleepPin;

	// Put Ultrasonic Sensors to sleep
	digitalWrite(sleep_pin, LOW);
  
	// Schedule Measurement
	scheduledFunc = MEASURE;
	time_ms = currTime_ms + 100 + millis() - startTime;
  
	return true;
}


void UltrasonicAnem::startNextFunc(uint64_t currTime_ms) {
	switch (scheduledFunc) {
		case MEASURE:
			measure(currTime_ms);
			break;
	}
}


void UltrasonicAnem::measure(uint64_t currTime_ms) {
	uint32_t startTime = millis();
	lastMeasurement = currTime_ms;
	measurement_ready = false;

	// Wakeup Ultrasonic Sensors
	if (debug) Serial.println("Ultrasonic Anem: Waking up ultrasonic sensors");
	digitalWrite(sleep_pin, HIGH);
	
	// Send chirps
	if (debug) Serial.println("Ultrasonic Anem: Sending chirps");
	unsigned long t1 = pulse(tx1, rx1);
	sleep(10);
	unsigned long t2 = pulse(tx2, rx2);
	
	// Calculate wind and temperature
	if (debug) Serial.println("Ultrasonic Anem: Calculating values");
	wind = (d / (sin_alpha * cos_alpha)) * ((1.0 / t1) - (1.0 / t2));
	temp = (d / sin_alpha) * ((1.0 / t1) + (1.0 / t2));

	// Put Ultrasonic Sensors to sleep
	if (debug) Serial.println("Ultrasonic Anem: Putting ultrasonic sensors to sleep");
	digitalWrite(sleep_pin, LOW);
	
	// Schedule new measurement
	measurement_ready = true;
	scheduledFunc = MEASURE;
	time_ms = lastMeasurement + period_ms;
	
}


unsigned long UltrasonicAnem::pulse(int trig, int echo) {
	// Send a chirp and determine time until response
	digitalWrite(trig, LOW);
	delayMicroseconds(2);
	digitalWrite(trig, HIGH);
	delayMicroseconds(10);
	digitalWrite(trig, LOW);
  
	return pulseIn(echo, HIGH);
}