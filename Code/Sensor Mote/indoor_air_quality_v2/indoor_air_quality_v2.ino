#include <sgp30.h>
#include <sps30.h>
#include <IpMtWrapper.h>


#define powerTest false
#define anemOnly true


#define SCL 9
#define SDA 10
#define LB 27


#define ANEM_SLEEP_PIN 5   // Set Anemometer sleep pin to 5 to enable, 0 to disable
#define PM_SLEEP_PIN 6    // Set PM sleep pin to 6 to enable, 0 to disable
#define CO2_SLEEP_PIN 7   // Set CO2 sleep pin to 7 to enable, 0 to disable


#define BATTERY_LIFE 2    // Set to 0 for 1 year, 1 for 6 months, or 2 for 3 months


#define SPS_FP true		// Set to true for PM sensor floating point values, false for 16-bit unsigned integers

// Uncomment this section for Climate Guard Anemometer
#include <cgAnem.h>
CG_Anem anem;
#define ANEM_OFFSET 0
//*/


/*// Uncomment this section for Ultrasonic Anemometer
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


IpMtWrapper smartmesh;
bool dataSent, smartmesh_init;


const uint64_t pm_periods[3][3] =   {{4320000, 11700000, 6300000},
                                     {2040000,  4320000, 2880000},
                                     { 960000,  1920000, 1320000}};
                                  
const uint64_t co2_periods[3][3] =  {{2220000, 10200000, 3060000},
                                     {1080000,  3660000, 1500000},
                                     { 510000,  1620000,  720000}};
                                  
const uint64_t anem_periods[3][3] = {{0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 40000},
                                     {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 20000},
                                     {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 10000}};


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

  if (dataSent && nextTime == 0xFFFFFFFFFFFFFFFF) {
    Serial.println("TEST DEV");
    nextTime = millis() + 10000;
    nextDevice = TEST;
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

  if (digitalRead(LB) == alertSent) {
    uint16_t alertVal = 0x6969, state = digitalRead(LB) ? 0x6969 : 0;
    memcpy(&payload[0], &alertVal, sizeof(uint16_t));
    memcpy(&payload[2], &state, sizeof(uint16_t));
    alertSent = !alertSent;
    return;
  }

  payload[0] = nextDevice;
  switch (nextDevice) {
    case CO2:
      memcpy(&payload[1], &co2.co2, sizeof(float));
      break;
    case PM:
      if (pm.fp) {
        memcpy(&payload[1], &pm.pm2p5_float, sizeof(float));
      } else {
        memcpy(&payload[1], &pm.pm2p5_int, sizeof(uint16_t));
      }
      break;
    case ANEM:
      memcpy(&payload[1], &anem.airflowRate, sizeof(float));
      break;
    case TEST:
      uint16_t testVal = 0xBEEF;
      memcpy(&payload[0], &testVal, sizeof(uint16_t));
      memcpy(&payload[2], &testVal, sizeof(uint16_t));
      break;
  }

  dataSent = true;
}


void setup() {
  Serial.begin(9600);
  Serial.println("Starting");

  for (int i = 1; i <= 40; i++) {
    digitalWrite(i, LOW);
    pinMode(i, OUTPUT);
  }

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

  // Initialize SmartMesh IP
  smartmesh_init = false;
  smartmesh.setup(60000, (uint8_t*)ipv6Addr_manager, 61000, 100, generateData, smartmesh);
  while (!smartmesh_init) smartmesh.loop();

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
    if (debug) Serial.println("Measurement Periods:");
    if (anem_present) {
      pm.period_ms = pm_periods[BATTERY_LIFE][1 + ANEM_OFFSET];
      co2.period_ms = co2_periods[BATTERY_LIFE][1 + ANEM_OFFSET];
      anem.period_ms = anem_periods[BATTERY_LIFE][1 + ANEM_OFFSET];
    } else {
      pm.period_ms = pm_periods[BATTERY_LIFE][0];
      co2.period_ms = co2_periods[BATTERY_LIFE][0];
      anem.period_ms = anem_periods[BATTERY_LIFE][0];
    }

    if (anem_present && anemOnly) {
      Serial.println("Anemometer only mode enabled");
      pm.period_ms = 0xFFFFFFFFFFFFFFFF;
      co2.period_ms = 0xFFFFFFFFFFFFFFFF;
      pm.time_ms = 0xFFFFFFFFFFFFFFFF;
      co2.time_ms = 0xFFFFFFFFFFFFFFFF;
      anem.period_ms = 60000;
    }

    if (debug) {
        Serial.print("PM: ");
        Serial.print((uint32_t)pm.period_ms);
        Serial.println(" ms");
        Serial.print("CO2: ");
        Serial.print((uint32_t)co2.period_ms);
        Serial.println(" ms");
        Serial.print("ANEM: ");
        Serial.print((uint32_t)anem.period_ms);
        Serial.println(" ms");
      }
  }

  if (debug) Serial.println("Sleeping to first device function");
  dataSent = true;
  nextDev();
  time_ms = nextTime;
  if (time_ms > millis()) {
    sleep(time_ms - millis());
  }
}

void loop() {
  if (digitalRead(LB) && alertSent) {
    if (debug) Serial.println("Battery Charged");
    dataSent = false;
    while (!dataSent) smartmesh.loop();
  }

  if (!digitalRead(LB) && !alertSent) {
    if (debug) Serial.println("ALERT: LOW BATTERY");
    dataSent = false;
    while (!dataSent) smartmesh.loop();
  } else {
    // Execute next function
    Serial.println("Started loop execution");
    uint32_t startTime = millis();

    if (debug) Serial.println("Performing device function");
    switch (nextDevice) {
      case CO2:
        if (debug) Serial.println("eCO2");
        if (anem_present && ANEM_SLEEP_PIN != 0 && ANEM_OFFSET == 0 && !anem.measurement_started) anem.startNextFunc(time_ms + startTime - millis());
        co2.startNextFunc(time_ms + startTime - millis());

        if (co2.measurement_ready) {
          dataSent = false;
          while (!dataSent) smartmesh.loop();
        }

        break;
      case PM:
        if (debug) Serial.println("PM");

        if (anem_present && ANEM_SLEEP_PIN != 0 && ANEM_OFFSET == 0 && !anem.measurement_started) anem.startNextFunc(time_ms + startTime - millis());
        if (CO2_SLEEP_PIN != 0) co2.sgp30_wakeup_transistor();
        pm.startNextFunc(time_ms + startTime - millis());
        if (pm.measurement_ready && CO2_SLEEP_PIN != 0) co2.sgp30_sleep_transistor();

        if (pm.measurement_ready) {
          dataSent = false;
          while (!dataSent) smartmesh.loop();
        }

        break;
      case ANEM:
        if (debug) Serial.println("Anemometer");

        if (CO2_SLEEP_PIN != 0) co2.sgp30_wakeup_transistor();
        anem.startNextFunc(time_ms + startTime - millis());
        if (anem.measurement_ready && CO2_SLEEP_PIN != 0) co2.sgp30_sleep_transistor();

        if (anem.measurement_ready) {
          dataSent = false;
          while (!dataSent) smartmesh.loop();
        }

        break;
      case TEST:
        dataSent = false;
        while (!dataSent) smartmesh.loop();
        break;
    }
  }

  if (co2.measurement_ready && pm.measurement_ready && anem.measurement_started) {
    anem.time_ms = time_ms;
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
  if (debug) Serial.println("Sleeping");
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
