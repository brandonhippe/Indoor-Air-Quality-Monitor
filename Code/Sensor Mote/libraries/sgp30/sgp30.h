// Include necessary libraries
#include <NewWire.h>
#include <stdint.h>
#include "crc.h"
#include "Energia.h"


// I2C Address and Function Selection definitions
#define ADDR 0x58

#define CALIBRATION 0
#define SET_CALIBRATION 1
#define GETCO2 2


// Declare SGP30 class
class SGP30 {
	public:
		// Public Variables
		uint32_t period_ms, max_clock;
		uint64_t time_ms;
		uint16_t co2, tvoc;
		boolean measurement_ready, debug;
		
		// Public Functions
		SGP30();
		boolean begin(boolean _debug);
		void startNextFunc(uint64_t currTime_ms);
	private:
		// Private Variables
		// uint16_t iaq_init, measure_iaq, get_iaq_baseline, set_iaq_baseline;
		uint64_t lastMeasurement;
		uint16_t baseline_CO2, baselineTVOC;
		int scheduledFunc, checks;
		boolean measurementStarted;
		
		// Private Functions
		void sgp30_sleep();
		void calibration(uint64_t currTime_ms);
		void setCalibration(uint64_t currTime_ms);
		void getCO2(uint64_t currTime_ms);
};