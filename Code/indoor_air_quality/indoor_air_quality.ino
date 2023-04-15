#include <sgp30.h>
#include <sps30.h>
#include <CGAnem.h>
#include <NewWire.h>


#define SDA_PIN 10    // set the SDA pin to 10
#define SCL_PIN 9    // set the SCL pin to 9
#define CG_SLEEP_PIN 8 // set Anemometer sleep pin to 8


uint64_t time_ms;


SGP30 sgp30;
SPS30 sps30;
CGAnem cganem;


int nextDevice;
#define CO2 0
#define PM 1
#define ANEM 2


void setup() {
	for (int i = 0; i < 40; i++) {
		digitalWrite(i, LOW);
		pinMode(i, OUTPUT);
	}

	Serial.begin(9600);
	Serial.println("Starting");

	pinMode(SDA_PIN, INPUT_PULLUP);
	pinMode(SCL_PIN, INPUT_PULLUP);

	int max_clock = 400000;
	if (sgp30.max_clock < max_clock) {
		max_clock = sgp30.max_clock;
	}

	if (sps30.max_clock < max_clock) {
		max_clock = sps30.max_clock;
	}
	
	if (cganem.max_clock < max_clock) {
		max_clock = cganem.max_clock;
	}

	Wire.setModule(0);
	Wire.begin();
	Wire.setClock(max_clock);
	sps30.begin(millis(), true);
	sps30.begin(MCPM2p5, millis(), true);
	cganem.begin(CG_SLEEP_PIN, millis(), true);

	// Set time_ms and delay to start time
	time_ms = sgp30.time_ms;
	while (millis() < time_ms);
}

void loop() {
	// Execute next function
	uint32_t startTime = millis();
	switch (nextDevice) {
		case CO2:
			sgp30.startNextFunc(time_ms + startTime - millis());
			break;
		case PM:
			sps30.startNextFunc(time_ms + startTime - millis());
			break;
	}

	// Print out measurements
	switch (nextDevice) {
		case CO2:
			if (sgp30.measurement_ready) {
				Serial.print("eCO2: ");
				Serial.print(sgp30.co2);
				Serial.println(" ppm");
			}
			break;
		case PM:
			if (sps30.measurement_ready) {
				switch (sps30.measurementIx) {
					case MCPM2p5:
						Serial.print("PM2.5 ");
						Serial.print(sps30.pm2p5);
						Serial.println(" ug/m3");
						break;
					case NCPM2p5:
						Serial.print("PM2.5 ");
						Serial.print(sps30.pm2p5);
						Serial.println(" per cm3");
						break;
				}
			}
			break;
	}

	// Execution finished, find next function
	// *** CURRENTLY ONLY SPS30 and SGP30 implemented, have to add anemometer (conditionally) and communication ***
	uint64_t nextTime = sgp30.time_ms;
	nextDevice = CO2;
  
	if (sps30.time_ms < nextTime) {
		nextTime = sps30.time_ms;
		nextDevice = PM;
	}

	// Calculate time until next function
	// Check if millis() has overflowed
	uint32_t currTime_32 = millis();
	uint64_t currTime_ms = (time_ms + 0xFFFFFFFF00000000) + currTime_32;
	if (currTime_ms < time_ms) {
		currTime_ms += 1 << 32;
	}

	// If next time hasn't passed, sleep until then
	if (nextTime > currTime_ms + (millis() - currTime_32)) {
		sleep(nextTime - (currTime_ms + (millis() - currTime_32)));
		time_ms = nextTime;
	} else {
		time_ms = currTime_ms + (millis() - currTime_32);
	}
}
