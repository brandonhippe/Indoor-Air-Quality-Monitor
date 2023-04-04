#ifndef SPS30
#define SPS30


// Include necessary libraries
#include <Wire.h>
#include <stdint.h>
#include "crc.h"
#include "msp430.h"


// Function Definitions
void sps30_sleep();
void sps30_setup(uint64_t currTime_ms);
uint16_t *sps30_getVal(int measurement[], int numVals);
void sps30_startMeasurement(uint64_t currTime_ms);
void sps30_highConcen_check(uint64_t currTime_ms);
void sps30_finalMeasurement(uint64_t currTime_ms);


// Declare I2C address and other necessary variables
#define ADDR 0x69

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

uint32_t sps30_period_ms = 60000;

static const uint16_t start_measurement = 0x0010, stop_measurement = 0x0014, data_ready = 0x0202, read_measured = 0x0300, start_sleep = 0x1001, wakeup = 0x1103;

void (*sps30_scheduledFunc)(uint64_t) = sps30_setup;
uint64_t sps30_time_ms;
static uint64_t lastMeasurement;

uint16_t sps30_pm2p5;


#endif
