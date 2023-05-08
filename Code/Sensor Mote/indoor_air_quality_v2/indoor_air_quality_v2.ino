#include <sgp30.h>
#include <sps30.h>
#include <NewWire.h>
#include <IpMtWrapper.h>


const int i2c_ports = 2;
const int i2c_pins[i2c_ports * 2] = {9, 10, 14, 15};
#define ANEM_SLEEP_PIN 8 	// Set Anemometer sleep pin to 8


#define SPS_FP true		// Set to true for PM sensor floating point values, false for 16-bit unsigned integers

// Uncomment this section for Climate Guard Anemometer
#include <CGAnem.h>
CGAnem anem;
//*/


/*// Uncomment this section for Ultrasonic Anemometer
#include <UltrasonicAnem.h>
UltrasonicAnem anem;
//*/


#define debug true


uint64_t time_ms, nextTime;


SGP30 co2;
SPS30 pm;


uint8_t nextDevice;
#define CO2 0
#define PM 1
#define ANEM 2


bool anem_present;


IpMtWrapper smartmesh;
bool dataSent, smartmesh_init;


void nextDev() {
    nextTime = co2.time_ms;
	nextDevice = CO2;
  
	if (pm.time_ms < nextTime) {
		nextTime = pm.time_ms;
		nextDevice = PM;
	}

	if (anem_present && anem.time_ms < nextTime) {
		nextTime = anem.time_ms;
		nextDevice = ANEM;
	}
}


void generateData(uint8_t* payload) {
    if (!smartmesh_init) {
		uint16_t testVal = 0xBEEF;
        memcpy(&payload[0], &testVal, sizeof(uint16_t));
        memcpy(&payload[2], &testVal, sizeof(uint16_t));
        smartmesh_init = true;
        return;
    }

    payload[0] = nextDevice;
    switch (nextDevice) {
        case CO2:
            memcpy(&payload[1], &co2.co2, sizeof(uint16_t));
            break;
        case PM:
			if (pm.fp) {
            	memcpy(&payload[1], &pm.pm2p5_float, sizeof(float));
			} else {
				memcpy(&payload[1], &pm.pm2p5_int, sizeof(uint16_t));
			}
            break;
        case ANEM:
            memcpy(&payload[1], &anem.wind, sizeof(uint16_t));
            break;
    }

    dataSent = true;
}


void setup() {
	for (int i = 0; i < 40; i++) {
		digitalWrite(i, i == ANEM_SLEEP_PIN);
		pinMode(i, OUTPUT);
	}

  	if (debug) {
  		Serial.begin(9600);
  		Serial.println("Starting");
  	}

	for (int i = 0; i < i2c_ports * 2; i++) {
		pinMode(i2c_pins[i], INPUT_PULLUP);
	}

	int max_clock = 400000;
	if (co2.max_clock < max_clock) {
		max_clock = co2.max_clock;
	}

	if (pm.max_clock < max_clock) {
		max_clock = pm.max_clock;
	}
	
	if (anem.max_clock < max_clock) {
		max_clock = anem.max_clock;
	}

	Wire.setModule(0);
	Wire.begin();
	Wire.setClock(max_clock);

    // Initialize SmartMesh IP
    smartmesh_init = false;
    smartmesh.setup(60000, (uint8_t*)ipv6Addr_manager, 61000, 100, generateData, smartmesh);
    while (!smartmesh_init) smartmesh.loop();

    // Initialize CO2 Sensor
    bool sensed = co2.begin(debug);
    if (!sensed) {
        if (debug) Serial.println("ERROR: CO2 sensor not connected!");
        while (1);
    }

    // Initialize PM Sensor
	sensed = pm.begin(MCPM2p5, SPS_FP, debug);
    if (!sensed) {
        if (debug) Serial.println("ERROR: PM sensor not connected!");
        while (1);
    }

	// Initialize Anemometer, if present
	anem_present = anem.begin(ANEM_SLEEP_PIN, debug);
    if (debug) {
        if (anem_present) {
            Serial.println("Anemometer connected!");
        } else {
            Serial.println("Anemometer not connected!");
        }
    }

    nextDev();
	time_ms = nextTime;
	while (millis() < time_ms);
}

void loop() {
	// Execute next function
	uint32_t startTime = millis();
  
	switch (nextDevice) {
		case CO2:
			if (debug) Serial.println("eCO2");
			co2.startNextFunc(time_ms + startTime - millis());

            if (co2.measurement_ready) {
                dataSent = false;
                while (!dataSent) smartmesh.loop();
            }

			break;
		case PM:
			if (debug) Serial.println("PM");
			pm.startNextFunc(time_ms + startTime - millis());

            if (pm.measurement_ready) {
                dataSent = false;
                while (!dataSent) smartmesh.loop();
            }

			break;
    	case ANEM:
			if (debug) Serial.println("Anemometer");
      		anem.startNextFunc(time_ms + startTime - millis());

            if (anem.measurement_ready) {
                dataSent = false;
                while (!dataSent) smartmesh.loop();
            }

      		break;
	}

	// Print out measurements
	if (debug) {
		switch (nextDevice) {
			case CO2:
				if (co2.measurement_ready) {
					Serial.print("eCO2: ");
					Serial.print(co2.co2);
					Serial.println(" ppm");
				}
				break;
			case PM:
				if (pm.measurement_ready) {
					switch (pm.measurementIx) {
						case MCPM2p5:
							Serial.print("PM2.5 ");
							if (pm.fp) {
								Serial.print(pm.pm2p5_float);
							} else {
								Serial.print(pm.pm2p5_int);
							}
							Serial.println(" ug/m3");
							break;
						case NCPM2p5:
							Serial.print("PM2.5 ");
							if (pm.fp) {
								Serial.print(pm.pm2p5_float);
							} else {
								Serial.print(pm.pm2p5_int);
							}
							Serial.println(" per cm3");
							break;
					}
				}
				break;
			case ANEM:
				if (anem.measurement_ready) {
					Serial.print("Airflow: ");
					Serial.print(anem.wind);
					Serial.println(" m/s");
				}
				break;
		}
	}

	// Execution finished, find next function
	nextDev();

	// Calculate time until next function
	// Check if millis() has overflowed
	uint32_t currTime_32 = millis();
	uint64_t currTime_ms = (time_ms & 0xFFFFFFFF00000000) + currTime_32;
	if (currTime_ms < time_ms) {
    if (debug) Serial.println("millis() overflowed!");
		currTime_ms += 0x0000000100000000;
	}

	// If next time hasn't passed, sleep until then
  	if (debug) Serial.println("Sleeping\n");
	if (nextTime > currTime_ms + (millis() - currTime_32)) {
		sleep(nextTime - (currTime_ms + (millis() - currTime_32)));
		time_ms = nextTime;
	} else {
		time_ms = currTime_ms + (millis() - currTime_32);
	}
}
