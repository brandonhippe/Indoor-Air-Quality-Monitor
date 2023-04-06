#include <sgp30.h>
#include <Wire.h>


uint64_t time_ms;


SGP30 sgp30;


void setup() {
  for (int i = 0; i < 40; i++) {
    digitalWrite(i, LOW);
    pinMode(i, OUTPUT);
  }

  Serial.begin(9600);
  Wire.begin();
  sgp30.begin(millis());

  // Set time_ms and delay to start time
  time_ms = sgp30.time_ms;
  while (millis() < time_ms);
}

void loop() {
  // Execute next function
  sgp30.startNextFunc(millis());

  // Sleep until next function
  sleep(sgp30.time_ms - millis());
}
