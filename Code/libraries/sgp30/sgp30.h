// Include necessary libraries
#include <Wire.h>
#include <stdint.h>
#include "crc.h"
#include "Energia.h"


// I2C Address and Function Selection definitions
#define ADDR 0x58

#define CALIBRATION 0
#define GETCO2 1


// Declare SGP30 class
class SGP30 {
	public:
		// Public Variables
		uint32_t period_ms;
		uint64_t time_ms;
		uint16_t co2;
		
		// Public Functions
		SGP30();
		boolean begin(uint64_t currTime_ms);
		void startNextFunc(uint64_t currTime_ms);
	private:
		// Private Variables
		uint16_t iaq_init, measure_iaq, get_iaq_baseline, set_iaq_baseline;
		uint16_t baseline_CO2, baselineTVOC;
		int calibrationMeasurements, scheduledFunc;
		
		// Private Functions
		void calibration(uint64_t currTime_ms);
		void getCO2(uint64_t currTime_ms);
};