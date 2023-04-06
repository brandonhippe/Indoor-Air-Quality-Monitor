#include <sgp30.h>
#include <sps30.h>
#include <Wire.h>


uint64_t time_ms;


SGP30 sgp30;
SPS30 sps30;


int nextDevice;
#define CO2 0
#define PM 1


void setup() {
  for (int i = 0; i < 40; i++) {
    digitalWrite(i, LOW);
    pinMode(i, OUTPUT);
  }

  Serial.begin(9600);
  Wire.begin();
  sps30.begin(millis());
  sps30.begin(millis());

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
