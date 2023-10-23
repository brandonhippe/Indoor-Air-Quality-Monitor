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
		uint32_t max_clock;
		uint64_t period_ms, time_ms;
		uint16_t co2_readings[15], tvoc_readings[15], baseline_CO2, baselineTVOC;
		float co2, tvoc;
		boolean measurement_ready, debug;
		
		// Public Functions
		SGP30();
		boolean begin(boolean _debug, int transistor_sleep);
		boolean begin(boolean _debug, int transistor_sleep, int co2Baseline, int tvocBaseline);
		void startNextFunc(uint64_t currTime_ms);
		void sgp30_wakeup_transistor();
		void sgp30_sleep_transistor();
		void calibration(uint64_t currTime_ms);
	private:
		// Private Variables
		// uint16_t iaq_init, measure_iaq, get_iaq_baseline, set_iaq_baseline;
		uint64_t lastMeasurement;
		int scheduledFunc, checks, transistor_pin;
		boolean measurement_started;
		
		// Private Functions
		void sgp30_sleep_I2C();
		void sgp30_wakeup_I2C();
		void setCalibration(uint64_t currTime_ms);
		void getCO2(uint64_t currTime_ms);
};