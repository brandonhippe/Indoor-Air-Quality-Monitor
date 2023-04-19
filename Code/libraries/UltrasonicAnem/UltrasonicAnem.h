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
		uint32_t period_ms, max_clock; // max_clock is unused, purely for I2C compatibility
		uint64_t time_ms;
		double wind, temp;
		int tx1, rx1, tx2, rx2;
		boolean measurement_ready, debug;
		
		// Public Functions
		UltrasonicAnem();
		boolean begin(int sleepPin, uint64_t currTime_ms, boolean _debug);
		void startNextFunc(uint64_t currTime_ms);
	private:
		// Private Variables
		uint64_t lastMeasurement;
		int scheduledFunc, sleep_pin;
		boolean measurementStarted;
		
		// Private Functions
		void measure(uint64_t currTime_ms);
		unsigned long pulse(int trig, int echo);
};