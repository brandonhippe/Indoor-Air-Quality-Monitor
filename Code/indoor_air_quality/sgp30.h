#ifndef SGP30
#define SGP30


// Include necessary libraries
#include <Wire.h>
#include <stdint.h>
#include "crc.h"
#include "msp430.h"


// Function definitions
void sgp30_setup();
void sgp30_setup_start(uint64_t currTime_ms);
void sgp30_calibration(uint64_t currTime_ms);
void sgp30_getCO2(uint64_t currTime_ms);


// Declare I2C address and other necessary variables
#define ADDR 0x58

uint32_t sgp30_period_ms = 60000;

static const uint16_t iaq_init = 0x2003, measure_iaq = 0x2008, get_iaq_baseline = 0x2015, set_iaq_baseline = 0x201e;
static uint16_t baseline_CO2, baselineTVOC;
static int calibrationMeasurements = 0;

void (*sgp30_scheduledFunc)(uint64_t) = sgp30_setup_start;
uint64_t sgp30_time_ms;

uint16_t sgp30_co2;


#endif
