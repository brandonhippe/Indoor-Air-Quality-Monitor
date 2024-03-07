#include <sgp30.h>
#include <sps30.h>


#define powerTest false


#define SCL 9
#define SDA 10
#define LB 27


#define ANEM_SLEEP_PIN 5   // Set Anemometer sleep pin to 5 to enable, 0 to disable
#define PM_SLEEP_PIN 6    // Set PM sleep pin to 6 to enable, 0 to disable
#define CO2_SLEEP_PIN 7   // Set CO2 sleep pin to 7 to enable, 0 to disable


#define BATTERY_LIFE 2    // Set to 0 for 1 year, 1 for 6 months, or 2 for 3 months


#define SPS_FP true		// Set to true for PM sensor floating point values, false for 16-bit unsigned integers

/*// Uncomment this section for Climate Guard Anemometer
  #include <cgAnem.h>
  CG_Anem anem;
  #define ANEM_OFFSET 0
  //*/


// Uncomment this section for Ultrasonic Anemometer
#include <UltrasonicAnem.h>
UltrasonicAnem anem;
#define ANEM_OFFSET 1
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


const uint64_t pm_periods[3][3] =   {{4320000, 15600000, 6300000},
                                     {2040000,  6600000, 2880000},
                                     { 960000,  2820000, 1320000}};
                                  
const uint64_t co2_periods[3][3] =  {{2220000, 13200000, 3060000},
                                     {1080000,  5400000, 1500000},
                                     { 510000,  2400000,  720000}};
                                  
const uint64_t anem_periods[3][3] = {{0xFFFFFFFFFFFFFFFF, 5160000, 40000},
                                     {0xFFFFFFFFFFFFFFFF, 2160000, 20000},
                                     {0xFFFFFFFFFFFFFFFF,  960000, 10000}};


bool alertSent = false;


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
  Serial.begin(9600);
  Serial.println("Starting");
  
  for (int i = 1; i <= 40; i++) {
    digitalWrite(i, LOW);
    pinMode(i, OUTPUT);
  }

  pinMode(1, INPUT);
  pinMode(21, INPUT);

  pinMode(SCL, INPUT_PULLUP);
  pinMode(SDA, INPUT_PULLUP);
  pinMode(LB, INPUT_PULLUP);

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

  // Initialize PM Sensor
  sensed = pm.begin(MCPM2p5, SPS_FP, debug, PM_SLEEP_PIN);
  if (!sensed) {
    if (debug) Serial.println("ERROR: PM sensor not connected!");
    while (!powerTest);
  }

  // Initialize CO2 Sensor
  sensed = co2.begin(debug, CO2_SLEEP_PIN);
  if (!sensed) {
    if (debug) Serial.println("ERROR: CO2 sensor not connected!");
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

  // Set Measurement Periods
  if (!powerTest) {
    if (anem_present) {
      pm.period_ms = pm_periods[BATTERY_LIFE][1 + ANEM_OFFSET];
      co2.period_ms = co2_periods[BATTERY_LIFE][1 + ANEM_OFFSET];
      anem.period_ms = anem_periods[BATTERY_LIFE][1 + ANEM_OFFSET];
      Serial.println((uint32_t)pm.period_ms);
      Serial.println((uint32_t)co2.period_ms);
      Serial.println((uint32_t)anem.period_ms);
    } else {
      pm.period_ms = pm_periods[BATTERY_LIFE][0];
      co2.period_ms = co2_periods[BATTERY_LIFE][0];
      anem.period_ms = anem_periods[BATTERY_LIFE][0];
      Serial.println((uint32_t)pm.period_ms);
      Serial.println((uint32_t)co2.period_ms);
    }
  }

  if (debug) Serial.println("Sleeping to first device function");
  nextDev();
  time_ms = nextTime;
  if (time_ms > millis()) {
    sleep(time_ms - millis());
  }
}

void loop() {
  if (digitalRead(LB) && alertSent) {
    alertSent = false;
    if (debug) Serial.println("Battery Charged");
  }
  
  if (!digitalRead(LB) && !alertSent) {
    alertSent = true;
    if (debug) Serial.println("ALERT: LOW BATTERY");
  } else {
    // Execute next function
    Serial.println("Started loop execution");
    uint32_t startTime = millis();
  
    if (debug) Serial.println("Performing device function");
    switch (nextDevice) {
      case CO2:
        if (debug) Serial.println("eCO2");
  
        if (anem_present && ANEM_SLEEP_PIN != 0 && ANEM_OFFSET == 0) anem.cg_wakeup();
        co2.startNextFunc(time_ms + startTime - millis());
        if (co2.measurement_ready && anem_present && ANEM_SLEEP_PIN != 0 && ANEM_OFFSET == 0) anem.cg_sleep();
  
        break;
      case PM:
        if (debug) Serial.println("PM");
  
        if (anem_present && ANEM_SLEEP_PIN != 0 && ANEM_OFFSET == 0) anem.cg_wakeup();
        if (CO2_SLEEP_PIN != 0) co2.sgp30_wakeup_transistor();
        pm.startNextFunc(time_ms + startTime - millis());
        if (pm.measurement_ready && anem_present && ANEM_SLEEP_PIN != 0 && ANEM_OFFSET == 0) anem.cg_sleep();
        if (pm.measurement_ready && CO2_SLEEP_PIN != 0) co2.sgp30_sleep_transistor();
        break;
      case ANEM:
        if (debug) Serial.println("Anemometer");
  
        if (CO2_SLEEP_PIN != 0) co2.sgp30_wakeup_transistor();
        anem.startNextFunc(time_ms + startTime - millis());
        if (anem.measurement_ready && CO2_SLEEP_PIN != 0) co2.sgp30_sleep_transistor();
  
        break;
    }
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
    uint32_t sleepTime = nextTime - (currTime_ms + (millis() - currTime_32));
    if (debug) Serial.println(sleepTime);
    uint64_t i;
    for (i = 0xFFFFFFFFFFFFFFFF; (i + 65536) < sleepTime && (digitalRead(LB) || alertSent); i += 65536) {
      if (debug) Serial.println((uint32_t)i);
      sleep(65536);
    }

    if (i + 65536 < sleepTime) {
      time_ms = nextTime - sleepTime + i + 1;
    } else {
      sleep(sleepTime % 65536);
      time_ms = nextTime;
    }
  } else {
    time_ms = currTime_ms + (millis() - currTime_32);
  }

  Serial.println();
}
