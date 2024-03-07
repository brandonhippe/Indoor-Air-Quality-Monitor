// Include necessary libraries
#include <NewWire.h>
#include <stdint.h>
#include "crc.h"
#include "Energia.h"


// I2C Address and Function Selection definitions
#define ADDR 0x62

#define GET_CO2 0
#define START_MEASUREMENT 1


#define HP_MODE 0   // High performance
#define LP_MODE 1   // Low power
#define SS_MODE 2   // Single Shot
#define PC_MODE 3   // Power-cycled single shot


// Declare SCD41 class
class SCD41 {
	public:
		// Public Variables
		int MODE;
		uint32_t max_clock;
		uint64_t period_ms, time_ms;
		float co2, temp, rh;
		boolean measurement_ready, debug;
		
		// Public Functions
		SCD41();
		boolean begin(boolean _debug, int mode, int transistor_sleep);
		void startNextFunc(uint64_t currTime_ms);
		void scd41_wakeup_transistor();
		void scd41_sleep_transistor();
		void setPeriod(uint64_t newPeriod_ms);
	private:
		// Private Variables
		uint64_t lastMeasurement;
		int scheduledFunc, checks, transistor_pin;
		boolean measurement_started;
		
		// Private Functions
		void scd41_sleep_I2C();
		void scd41_wakeup_I2C();
		void getCO2(uint64_t currTime_ms);
};