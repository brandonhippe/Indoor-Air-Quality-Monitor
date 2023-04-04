uint64_t time_ms;

#include "sgp30.h"
#include "sps30.h"
#include <Wire.h>


void (*nextFunc)(uint64_t);


void setup() {
  for (int i = 0; i < 40; i++) {
    digitalWrite(i, LOW);
    pinMode(i, OUTPUT);
  }

  Serial.begin(9600);
  Wire.begin();

  // Schedule initialization functions
  time_ms = millis();
  nextFunc = sps30_scheduledFunc;
  sps30_time_ms = time_ms + 100 + time_ms - millis();
  sps30_time_ms = time_ms + 200 + time_ms - millis();

  // Set time_ms and delay to start time
  time_ms = sps30_time_ms;
  while (millis() < time_ms);
}

void loop() {
  // Execute next function
  nextFunc(time_ms);

  // Execution finished, find next function
  // *** CURRENTLY ONLY SPS30 and SGP30 implemented, have to add anemometer (conditionally) and communication ***
  uint64_t nextTime = sgp30_time_ms;
  nextFunc = sgp30_scheduledFunc;
  
  if (sps30_time_ms < nextTime) {
    nextTime = sps30_time_ms;
    nextFunc = sps30_scheduledFunc;
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
