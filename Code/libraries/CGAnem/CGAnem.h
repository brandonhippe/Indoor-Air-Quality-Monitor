// Include necessary libraries
#include <NewWire.h>
#include <stdint.h>
#include "Energia.h"


// I2C Address and Function Selection definitions
#define ADDR 0x11

#define START_MEASUREMENT 0
#define FINISH_MEASUREMENT 1



// Declare CGAnem class
class CGAnem {
	public:
		// Public Variables
		uint32_t period_ms, max_clock;
		uint64_t time_ms;
		uint16_t wind;
		boolean measurement_ready, debug;
		
		// Public Functions
		CGAnem();
		boolean begin(int sleepPin, boolean _debug);
		void startNextFunc(uint64_t currTime_ms);
	private:
		// Private Variables
		uint64_t lastMeasurement;
		int scheduledFunc, sleep_pin;
		
		// Private Functions
		void start_measurement(uint64_t currTime_ms);
		void finish_measurement(uint64_t currTime_ms);
};