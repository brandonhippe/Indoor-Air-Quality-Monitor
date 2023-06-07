// Include necessary libraries
#include <stdint.h>
#include <math.h>
#include "Energia.h"


// Function Selection definitions
#define MEASURE 0


// Declare UltrasonicAnem class
class UltrasonicAnem {
	public:
		// Public Variables
		uint32_t max_clock; // max_clock is unused, purely for I2C compatibility
		uint64_t period_ms, time_ms;
		float airflowRate, temp;
		int TRIG_1, ECHO_1, TRIG_2, ECHO_2;
		boolean measurement_ready, debug;
		
		// Public Functions
		UltrasonicAnem();
		boolean begin(int sleepPin, boolean _debug);
		void startNextFunc(uint64_t currTime_ms);
		void cg_sleep();
    	void cg_wakeup();
	private:
		// Private Variables
		uint64_t lastMeasurement;
		int scheduledFunc, sleep_pin, measurements, normal1, normal2;
		float vent_flow, recorded[3];
		boolean measurementStarted;
		
		// Private Functions
		void measure(uint64_t currTime_ms);
		unsigned long pulse(int trig, int echo);
		boolean withinPercent(int t, int normal);
};