#include <UltrasonicAnem.h>


#define SLEEP_PIN 5


uint64_t time_ms;


UltrasonicAnem anem;


void setup() {
  for (int i = 0; i < 40; i++) {
    digitalWrite(i, LOW);
    pinMode(i, OUTPUT);
  }

  sleep(1000);

  Serial.begin(9600);
  Serial.println("Starting");
  
  
  bool sensed = anem.begin(SLEEP_PIN, true);
  if (!sensed) {
    Serial.println("ERROR: Ultrasonic Anemometer not connected!");
    while (1);
  }

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
	  Serial.print(anem.airflowRate);
	  Serial.println(" m/s");
  }

  // Sleep until next function
  Serial.println("Sleeping");
  sleep(anem.time_ms - millis());
}
