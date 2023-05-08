#include <UltrasonicAnem.h>


#define SLEEP_PIN 8


uint64_t time_ms;


UltrasonicAnem anem;


void setup() {
  for (int i = 0; i < 40; i++) {
    digitalWrite(i, LOW);
    pinMode(i, OUTPUT);
  }

  Serial.begin(9600);
  Serial.println("Starting");
  
  
  anem.begin(SLEEP_PIN, millis(), true);

  // Set time_ms and delay to start time
  time_ms = anem.time_ms;
  while (millis() < time_ms);
}

void loop() {
  // Execute next function
  Serial.println("Executing");
  anem.startNextFunc(millis());
  if (anem.measurement_ready) {
    Serial.print("Wind: ");
	Serial.print(anem.wind);
	Serial.println(" m/s");
  }

  // Sleep until next function
  Serial.println("Sleeping");
  sleep(anem.time_ms - millis());
}
