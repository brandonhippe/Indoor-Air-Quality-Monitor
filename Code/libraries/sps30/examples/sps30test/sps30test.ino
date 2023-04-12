#include <sps30.h>
#include <Wire.h>


uint64_t time_ms;


SPS30 sps30;


void setup() {
  for (int i = 0; i < 40; i++) {
    digitalWrite(i, LOW);
    pinMode(i, OUTPUT);
  }

  Serial.begin(9600);
  Serial.println("Starting");
  Wire.begin();
  sps30.begin(millis());

  // Set time_ms and delay to start time
  time_ms = sps30.time_ms;
  while (millis() < time_ms);
}

void loop() {
  // Execute next function
  Serial.println("Measuring");
  sps30.startNextFunc(millis());
  Serial.println(sps30.pm2p5);

  // Sleep until next function
  Serial.println("Sleeping");
  sleep(sps30.time_ms - millis());
}
