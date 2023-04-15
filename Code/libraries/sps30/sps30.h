// Include necessary libraries
#include <NewWire.h>
#include <stdint.h>
#include "crc.h"
#include "Energia.h"


// I2C Address, Function Selection, and Measurement Type definitions
#define ADDR 0x69

#define START_MEASUREMENT 0
#define CONCENTRATION_CHECK 1
#define FINISH_MEASUREMENT 2

#define MCPM1p0 0
#define MCPM2p5 1
#define MCPM4p0 2
#define MCPM10 3
#define NCPM0p5 4
#define NCPM1p0 5
#define NCPM2p5 6
#define NCPM4p0 7
#define NCPM10 8
#define PART_SIZE 9


class SPS30 {
	public:
		// Public Variables
		uint32_t period_ms, max_clock;
		uint64_t time_ms;
		uint16_t pm2p5;
		boolean measurement_ready, debug;
		int measurementIx;
		
		// Public Functions
		SPS30();
		boolean begin(int measurement, uint64_t currTime_ms, boolean _debug);
		void startNextFunc(uint64_t currTime_ms);
	private:
		// Private Variables
		// uint16_t start_measurement, stop_measurement, data_ready, read_measured, start_sleep, wakeup;
		uint64_t lastMeasurement;
		int scheduledFunc;
		
		// Private Functions
		void sps30_sleep();
		void startMeasurement(uint64_t currTime_ms);
		void highConcen_check(uint64_t currTime_ms);
		void finalMeasurement(uint64_t currTime_ms);
	
};