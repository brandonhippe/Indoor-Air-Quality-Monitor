#include <sgp30.h>
#include <sps30.h>


#define powerTest true


#define SCL 9
#define SDA 10


#define ANEM_SLEEP_PIN 5   // Set Anemometer sleep pin to 5
#define PM_SLEEP_PIN 0    // Set PM sleep pin to 0 to disable
#define CO2_SLEEP_PIN 7   // Set CO2 sleep pin to 0 to disable



#define SPS_FP true		// Set to true for PM sensor floating point values, false for 16-bit unsigned integers

/*// Uncomment this section for Climate Guard Anemometer
  #include <cgAnem.h>
  CG_Anem anem;
  //*/


// Uncomment this section for Ultrasonic Anemometer
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
#define TEST 3


bool anem_present;



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

  if (nextTime == 0xFFFFFFFFFFFFFFFF) {
    Serial.println("TEST DEV");
    nextTime = millis() + 10000;
    nextDevice = TEST;
  }
}


void setup() {
  for (int i = 2; i <= 40; i++) {
    if (i == 21) {
      continue;
    }
    
    digitalWrite(i, LOW);
    pinMode(i, OUTPUT);
  }

  if (debug) {
    Serial.begin(9600);
    Serial.println("Starting");
  }

  pinMode(SCL, INPUT_PULLUP);
  pinMode(SDA, INPUT_PULLUP);

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


  bool sensed = false;

  // Initialize CO2 Sensor
  sensed = co2.begin(debug, CO2_SLEEP_PIN);
  if (!sensed) {
    if (debug) Serial.println("ERROR: CO2 sensor not connected!");
    while (!powerTest);
  }

  // Initialize PM Sensor
  sensed = pm.begin(MCPM2p5, SPS_FP, debug, PM_SLEEP_PIN);
  if (!sensed) {
    if (debug) Serial.println("ERROR: PM sensor not connected!");
    while (!powerTest);
  }

  // Initialize Anemometer, if present
  anem_present = anem.begin(ANEM_SLEEP_PIN, debug);
  if (anem_present) {
    if (debug) Serial.println("Anemometer connected!");
  } else {
    if (debug) Serial.println("Anemometer not connected!");
    anem.time_ms = 0xFFFFFFFFFFFFFFFF;
  }

  if (debug) Serial.println("Sleeping to first device function");
  nextDev();
  time_ms = nextTime;
  if (time_ms > millis()) {
    sleep(time_ms - millis());
  }
}

void loop() {
  // Execute next function
  Serial.println("Started loop execution");
  uint32_t startTime = millis();

  if (debug) Serial.println("Performing device function");
  switch (nextDevice) {
    case CO2:
      if (debug) Serial.println("eCO2");
      co2.startNextFunc(time_ms + startTime - millis());
      break;
    case PM:
      if (debug) Serial.println("PM");
      pm.startNextFunc(time_ms + startTime - millis());
      break;
    case ANEM:
      if (debug) Serial.println("Anemometer");
      anem.startNextFunc(time_ms + startTime - millis());
      break;
  }

  // Print out measurements
  if (debug) {
    Serial.println("Device Function Finished");
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
          Serial.print(anem.airflowRate);
          Serial.println(" m/s");
        }
        break;
    }
  }

  // Execution finished, find next function
  if (debug) Serial.println("Finding next function");
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
