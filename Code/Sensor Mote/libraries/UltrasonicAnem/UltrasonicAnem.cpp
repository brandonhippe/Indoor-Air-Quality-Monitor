#include "UltrasonicAnem.h"


#define SLEEP_LOGIC true
const double a = 0.04, d = 0.05; // a (pitch) and d (height) in meters
double sin_alpha, cos_alpha;
unsigned long t1, t2;

double percent = 0.05;


UltrasonicAnem::UltrasonicAnem() {
	max_clock = 400000;
	period_ms = 0xFFFFFFFFFFFFFFFF;
	period_ms = 20000;
	measurement_ready = false;
	measurementStarted = false;
	TRIG_1 = 36;
	ECHO_1 = 17;
	TRIG_2 = 18;
	ECHO_2 = 19;
	
	// double alpha = atan(2 * d / a);
	// sin_alpha = sin(alpha);
	// cos_alpha = cos(alpha);

	vent_flow = 0.3;
}


boolean UltrasonicAnem::begin(int sleepPin, boolean _debug) {
	uint32_t startTime = millis();
	debug = _debug;
	sleep_pin = sleepPin;

	pinMode(ECHO_1, INPUT);
	pinMode(ECHO_2, INPUT);

	// Wake up Ultrasonic Sensors
	if (debug) Serial.println("Ultrasonic Anem: Initializing");
	digitalWrite(sleep_pin, !SLEEP_LOGIC);
	sleep(100);
	
	normal1 = pulse(TRIG_1, ECHO_1);
	if (debug) Serial.println(normal1);
	sleep(10);
	normal2 = pulse(TRIG_2, ECHO_2);
	if (debug) Serial.println(normal2);

	if (normal1 == 0 || normal2 == 0) {
		digitalWrite(sleep_pin, LOW);
		time_ms = 0xFFFFFFFFFFFFFFFF;
		return false;
	}

	// Put Ultrasonic Sensors to sleep
	digitalWrite(sleep_pin, SLEEP_LOGIC);
  
	// Schedule Measurement
	measurements = 0;
	scheduledFunc = MEASURE;
	time_ms = millis();
  
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
	measurement_ready = false;
	if (debug) Serial.println(measurements);
	if (measurements == 3) {
		lastMeasurement = currTime_ms;
		measurements = 0;
	}

	// Wakeup Ultrasonic Sensors
	if (debug) Serial.println("Ultrasonic Anem: Waking up ultrasonic sensors");
	digitalWrite(sleep_pin, !SLEEP_LOGIC);
	sleep(100);
	
	// Send chirps
	if (debug) Serial.println("Ultrasonic Anem: Sending chirps");
	t1 = pulse(TRIG_1, ECHO_1);
	if (debug) Serial.println(t1);
	sleep(10);
	t2 = pulse(TRIG_2, ECHO_2);
	if (debug) Serial.println(t2);
	
	// Calculate wind and temperature
	if (debug) Serial.println("Ultrasonic Anem: Calculating values");
	// airflowRate = (d / (sin_alpha * cos_alpha)) * ((1.0 / t1) - (1.0 / t2));
	// temp = (d / sin_alpha) * ((1.0 / t1) + (1.0 / t2));
	recorded[measurements] = vent_flow * (!withinPercent(t1, normal1) - !withinPercent(t2, normal2));
	Serial.println(recorded[measurements]);

	// Put Ultrasonic Sensors to sleep
	if (debug) Serial.println("Ultrasonic Anem: Putting ultrasonic sensors to sleep");
	digitalWrite(sleep_pin, SLEEP_LOGIC);
	sleep(5);

	measurements++;
	
	// Schedule new measurement
	measurement_ready = measurements == 3;
	scheduledFunc = MEASURE;
	if (measurements < 3) {
		time_ms = currTime_ms + 2000;
	} else {
		time_ms = lastMeasurement + period_ms;

		if (recorded[0] == recorded[1] || recorded[0] == recorded[2]) {
			airflowRate = recorded[0];
		} else if (recorded[1] == recorded[2]) {
			airflowRate = recorded[1];
		} else {
			airflowRate = 0;
		}
	}
	
}


unsigned long UltrasonicAnem::pulse(int trig, int echo) {
	// Send a chirp and determine time until response
	digitalWrite(trig, LOW);
	delayMicroseconds(2);
	digitalWrite(trig, HIGH);
	delayMicroseconds(10);
	digitalWrite(trig, LOW);
  
	return pulseIn(echo, HIGH, 10000);
}


boolean UltrasonicAnem::withinPercent(int t, int normal) {
	return (((1 - percent) * normal) <= t && t <= ((1 + percent) * normal));
}


void UltrasonicAnem::cg_wakeup() {
}


void UltrasonicAnem::cg_sleep() {
}